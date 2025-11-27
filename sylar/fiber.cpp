#include "fiber.h"


extern "C" void swap_context(sylar::FiberContext*, sylar::FiberContext*);

namespace sylar{

static thread_local FiberEnvironment t_fiber_env;

void swap_fiber(std::shared_ptr<Fiber> old_fiber, std::shared_ptr<Fiber> new_fiber);

FiberMem::FiberMem(size_t size):
    stack_size_(size),
    stack_buffer_(new char[size]){
    stack_bottom_ = stack_buffer_.get() + size;
}

void FiberMem::change_occupier(std::shared_ptr<Fiber> new_occupier){
    if(occupier_ == new_occupier || occupier_ == nullptr){
        occupier_ = new_occupier;
        return;
    }
    SYLAR_ASSERT(occupier_->get_state() == FiberState::SUSPENDED \
        || occupier_->get_state() == FiberState::TERMINATED);
    occupier_->save_fiber_stack();
    occupier_ = new_occupier;
}

FiberSharedStackPool::FiberSharedStackPool(size_t stack_count, size_t stack_size)
    :alloc_idx_(0)
    ,stack_size_(stack_size)
    ,stack_count_(stack_count),
    stack_array_(stack_count, std::shared_ptr<FiberMem>(new FiberMem(stack_size))){
    ;
}

std::shared_ptr<FiberMem> FiberSharedStackPool::get_fiber_stack(){
    auto stack = stack_array_[alloc_idx_];
    alloc_idx_ = (alloc_idx_ + 1) % stack_count_;
    return stack;
}




FiberEnvironment::FiberEnvironment():
    call_stack_depth_(0),
    fiber_call_stack_(SYLAR_FIBER_MAX_CALL_STACK_DEPTH),
    swap_in_fiber_(nullptr),
    swap_out_fiber_(nullptr){
    // init main fiber
    std::shared_ptr<Fiber> main_fiber(new Fiber);
    push_fiber(main_fiber);
}

void FiberEnvironment::push_fiber(std::shared_ptr<Fiber> fiber){
    fiber_call_stack_[call_stack_depth_] = fiber;
    ++call_stack_depth_;
}
void FiberEnvironment::pop_fiber(){
    if(call_stack_depth_ <= 0)
        return;
    --call_stack_depth_;
    fiber_call_stack_[call_stack_depth_].reset();
}

void make_context(std::shared_ptr<Fiber> fiber){
    FiberContext* ctx = &fiber->context_;
    char* sp = ctx->ss_sp + ctx->ss_size - sizeof(void*);
    sp = (char*)((uint64_t)sp & 0xFFFFFFFFFFFFFFF0);
    
    void** ret_addr = (void**)sp;
    *ret_addr = (void*)&Fiber::fiber_func_wrapper;

    memset(ctx->registers, 0, sizeof(ctx->registers));
    ctx->registers[FiberContext::RSP] = (uint64_t)sp;
    ctx->registers[FiberContext::RDI] = (uint64_t)fiber.get();
    ctx->rflags = 0x200;

    fiber->set_state(FiberState::READY);
}


Fiber::Fiber(fiber_func func, void* args, std::shared_ptr<FiberSharedStackPool> stack_poll, size_t stack_size):
    env_(t_fiber_env),
    is_main_fiber_(false),
    state_(FiberState::INITING),
    func_(func),
    func_args_(args){
    // shared stack
    if(stack_poll){
        use_shared_stack_ = true;
        std::shared_ptr<FiberMem> stack = stack_poll->get_fiber_stack();
        running_mem_ = stack;
    }
    // own stack
    else{
        use_shared_stack_ = false;
        if(stack_size > 1024 * 1024 * 8)
            stack_size = 1024 * 1024 * 8;
        else if(stack_size == 0)
            stack_size = SYLAR_FIBER_DEFAULT_STACK_SIZE;
        if(stack_size & 0xFFF){
            stack_size &= ~0xFFF;
            stack_size += 0x1000;
        }
        running_mem_ = std::make_shared<FiberMem>(stack_size);
    }

    context_.ss_size = running_mem_->get_stack_size();
    context_.ss_sp = running_mem_->get_stack_buffer();

    save_size_ = 0;
    save_buffer_ = nullptr;
}
Fiber::Fiber():
    env_(t_fiber_env),
    is_main_fiber_(true),
    state_(FiberState::RUNNING),
    func_(nullptr),
    func_args_(nullptr){
    use_shared_stack_ = false;
    running_mem_ = nullptr;

    context_.ss_size = 0;
    context_.ss_sp = nullptr;
    save_size_ = 0;
    save_buffer_ = nullptr;
}

void Fiber::fiber_resume(){
    if(get_state() == FiberState::INITING)
        make_context(shared_from_this());
    else if(get_state() == FiberState::TERMINATED)
        return;

    std::shared_ptr<Fiber> cur_fiber = env_.get_current_fiber();
    env_.push_fiber(shared_from_this());
    set_state(FiberState::RUNNING);
    cur_fiber->set_state(FiberState::SUSPENDED);
    std::cout << "resume: ready to swap_fiber\n";
    swap_fiber(cur_fiber, shared_from_this());
}
void Fiber::fiber_yield(){
    FiberEnvironment& env = t_fiber_env;
    std::shared_ptr<Fiber> cur_fiber = env.get_current_fiber();
    env.pop_fiber();
    std::shared_ptr<Fiber> next_fiber = env.get_current_fiber();
    if(cur_fiber->get_state() == FiberState::RUNNING)
        cur_fiber->set_state(FiberState::SUSPENDED);
    next_fiber->set_state(FiberState::RUNNING);
    swap_fiber(cur_fiber, next_fiber);
    std::cout << "fiber_yield back\n";
}

void Fiber::fiber_func_wrapper(Fiber* fiber){
    fiber->set_state(FiberState::RUNNING);
    fiber->func_(fiber->func_args_);
    
    fiber->set_state(FiberState::TERMINATED);
    fiber_yield();
}


void swap_fiber(std::shared_ptr<Fiber> old_fiber, std::shared_ptr<Fiber> new_fiber){
    std::cout << "swap_fiber" << std::endl;
    uintptr_t rsp_value = 0;
    asm volatile ("movq %%rsp, %0" : "=r" (rsp_value));
    old_fiber->stack_buffer_top_ = (char*)rsp_value;

    if(new_fiber->use_shared_stack_){
        t_fiber_env.swap_out_fiber_ = new_fiber->running_mem_->get_occupier();
        t_fiber_env.swap_in_fiber_ = new_fiber;
        new_fiber->running_mem_->change_occupier(new_fiber);
        // If the fiber we're about to swap in previously saved its stack, restore
        // it into the shared stack buffer before switching context. Restoring
        // before swap ensures the resumed fiber sees a correct stack image.
        if(new_fiber->save_buffer_ && new_fiber->save_size_ > 0){
            char* dest = new_fiber->running_mem_->get_stack_bottom() - new_fiber->save_size_;
            memcpy(dest, new_fiber->save_buffer_.get(), new_fiber->save_size_);
            new_fiber->stack_buffer_top_ = dest;
        }
    }
    else{
        t_fiber_env.swap_out_fiber_ = nullptr;
        t_fiber_env.swap_in_fiber_ = nullptr;
    }
    std::cout << "before swap_context" << std::endl;
    swap_context(&old_fiber->context_, &new_fiber->context_);
    std::cout << "after swap_context" << std::endl;

    // clear swap markers
    t_fiber_env.swap_in_fiber_ = nullptr;
    t_fiber_env.swap_out_fiber_ = nullptr;

    // resume stack
    // the new_fiber is different from the old_fiber,
    // because when swap back, the old_fiber is now suspended
    static std::shared_ptr<Fiber> b_new_fiber = t_fiber_env.swap_in_fiber_;
    static std::shared_ptr<Fiber> b_old_fiber = t_fiber_env.swap_out_fiber_;
    b_new_fiber = t_fiber_env.swap_in_fiber_;
    b_old_fiber = t_fiber_env.swap_out_fiber_;
    if(b_new_fiber && b_old_fiber && b_new_fiber != b_old_fiber){
        if(b_new_fiber->save_buffer_ && b_new_fiber->save_size_ > 0)
            memcpy(b_new_fiber->stack_buffer_top_, b_new_fiber->save_buffer_.get(), b_new_fiber->save_size_);
    }
}

void Fiber::save_fiber_stack(){
    std::cout << "save_fiber_stack\n";
    size_t stack_size = running_mem_->get_stack_bottom() - stack_buffer_top_;
    if(save_buffer_)
        save_buffer_.reset();
    save_size_ = stack_size;
    save_buffer_ = std::unique_ptr<char[]>(new char[save_size_]);
    memcpy(save_buffer_.get(), stack_buffer_top_, save_size_);
}

    
}