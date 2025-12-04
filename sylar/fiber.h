#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <ucontext.h>
#include <stdint.h>
#include <string.h>

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <stack>

#include "macro.h"
#include "type.h"

#define SYLAR_FIBER_DEFAULT_STACK_SIZE (128 * 1024)     // 128KB

#define SYLAR_FIBER_MAX_CALL_STACK_DEPTH  128

namespace sylar{
class Fiber;
class FiberContext;
class Scheduler;

// TODO: in some functions, we shouldn't use shared_ptr. 
// Because when the fiber is swapped out and never swapped in again,
// the shared_ptr will never be released, causing memory leak.

//TODO: use template for func args
//typedef void (*fiber_func)(void*);
typedef std::function<void(void*)> fiber_func;

enum class FiberState : int{
    INITING,
    READY,
    RUNNING,
    SUSPENDED,
    TERMINATED,
    EXCEPTION
};

// the stack structure of a fiber
class FiberMem{
public:
    FiberMem(size_t size = SYLAR_FIBER_DEFAULT_STACK_SIZE);
    
    char* get_stack_bottom() const { return stack_bottom_; }
    char* get_stack_buffer() const { return stack_buffer_.get(); }
    size_t get_stack_size() const { return stack_size_; }

    std::shared_ptr<Fiber> get_occupier() const { return occupier_; }
    void change_occupier(std::shared_ptr<Fiber> new_occupier);
private:
    std::shared_ptr<Fiber> occupier_;    // the fiber occupying this stack now
    size_t stack_size_;      // size of the stack
    std::unique_ptr<char[]> stack_buffer_;  // stack memory buffer
    char* stack_bottom_;     // stack_buffer + stack_size
};

class FiberSharedStackPool{
public:
    FiberSharedStackPool(size_t stack_count, size_t stack_size = SYLAR_FIBER_DEFAULT_STACK_SIZE);

    std::shared_ptr<FiberMem> get_fiber_stack();
private:
    size_t alloc_idx_;      // index for next allocation
    size_t stack_size_;     // size of each stack
    size_t stack_count_;    // number of stacks
    std::vector<std::shared_ptr<FiberMem>> stack_array_; // stacks that can be shared
};

// used to manage fiber environment per thread
class FiberEnvironment{
public:
    //init the env, init the main fiber
    FiberEnvironment();

    std::shared_ptr<Fiber> get_main_fiber() const { return fiber_call_stack_[0]; }
    std::shared_ptr<Fiber> get_current_fiber() const;
    void push_fiber(std::shared_ptr<Fiber> fiber);
    void pop_fiber();
private:
    friend void swap_fiber(std::shared_ptr<Fiber> old_fiber, std::shared_ptr<Fiber> new_fiber);
private:
    int call_stack_depth_;      // depth of the fiber call stack
    std::vector<std::shared_ptr<Fiber>> fiber_call_stack_;    // the call stack of fibers in this thread
    // used for swapping stacks when using shared stacks
    std::shared_ptr<Fiber> swap_in_fiber_;      // fiber to swap in
    std::shared_ptr<Fiber> swap_out_fiber_;     // fiber to swap out
};


struct alignas(16) FiberContext{
    enum REGISTER{
        RBX = 0, RBP = 1, RSP = 2, R12 = 3, R13 = 4, R14 = 5, R15 = 6, 
        RDI = 7, REG_COUNT
    };
    uint64_t registers[REG_COUNT];
    uint64_t rflags;
    uint64_t ret_addr;

    char* ss_sp;
    size_t ss_size;
};
static void make_context(std::shared_ptr<Fiber> fiber);


class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    Fiber(fiber_func func, void* args, std::shared_ptr<FiberSharedStackPool> stack_poll = nullptr, size_t stack_size = SYLAR_FIBER_DEFAULT_STACK_SIZE);

    bool reset(fiber_func func, void* args);

    void fiber_resume();
    static void fiber_yield();

    FiberState get_state() const { return state_; }
    fid_t get_id() const { return id_; }
    static std::shared_ptr<Fiber> get_this();
private:
    friend class Scheduler;
    friend FiberEnvironment::FiberEnvironment();
    friend void FiberMem::change_occupier(std::shared_ptr<Fiber> new_occupier);
    friend void make_context(std::shared_ptr<Fiber> fiber);
    friend void swap_fiber(std::shared_ptr<Fiber> old_fiber, std::shared_ptr<Fiber> new_fiber);
private:
    // init main fiber
    Fiber();
    void save_fiber_stack();
    static void fiber_func_wrapper(Fiber* fiber);
    void set_state(FiberState state) { state_ = state; }
    void set_env();
private:
    fid_t id_;
    FiberEnvironment* env_;
    bool is_main_fiber_;
    FiberState state_;
    fiber_func func_;
    void* func_args_;
    FiberContext context_;
    
    bool use_shared_stack_;
    std::shared_ptr<FiberMem> running_mem_; // stack in running state
    char* stack_buffer_top_;      // top of the stack in running state

    size_t save_size_;
    std::unique_ptr<char[]> save_buffer_;  // buffer the stack when the fiber is suspended

};


}


#endif