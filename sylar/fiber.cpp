#include "fiber.h"


namespace sylar{

static thread_local FiberEnvironment t_fiber_env;

FiberStack::FiberStack(size_t size):
    stack_size_(size),
    stack_buffer_(std::make_unique<char[]>(size)){
    stack_bottom_ = stack_buffer_.get() + size;
}

FiberSharedStackPool::FiberSharedStackPool(size_t stack_count, size_t stack_size)
    :alloc_idx_(0)
    ,stack_size_(stack_size)
    ,stack_count_(stack_count),
    stack_array_(stack_count, std::make_unique<FiberStack>(stack_size)){
    ;
}

std::shared_ptr<FiberStack> FiberSharedStackPool::get_fiber_stack(){
    auto stack = stack_array_[alloc_idx_];
    alloc_idx_ = (alloc_idx_ + 1) % stack_count_;
    return stack;
}




FiberEnvironment::FiberEnvironment():
    call_stack_depth_(0),
    fiber_call_stack_(SYLAR_FIBER_MAX_CALL_STACK_DEPTH){
    //TODO: init main fiber
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





    
}