#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <ucontext.h>

#include <memory>
#include <functional>
#include <vector>
#include <stack>

#define SYLAR_FIBER_DEFAULT_STACK_SIZE (128 * 1024)     // 128KB

#define SYLAR_FIBER_MAX_CALL_STACK_DEPTH  128


namespace sylar{

class Fiber;

enum class FiberState : int{
    READY,
    RUNNING,
    SUSPENDED,
    TERMINATED,
    EXCEPTION
};

// the stack structure of a fiber
class FiberStack{
public:
    FiberStack(size_t size = SYLAR_FIBER_DEFAULT_STACK_SIZE);
    
    const char* get_stack_bottom() const { return stack_bottom_; }
    const char* get_stack_top() const { return stack_bottom_ - stack_size_; }
    size_t get_stack_size() const { return stack_size_; }
private:
    std::shared_ptr<Fiber> occupier_;    // the fiber occupying this stack now
    size_t stack_size_;      // size of the stack
    std::unique_ptr<char[]> stack_buffer_;  // stack memory buffer
    char* stack_bottom_;     // stack_buffer + stack_size
};

class FiberSharedStackPool{
public:
    FiberSharedStackPool(size_t stack_count, size_t stack_size = SYLAR_FIBER_DEFAULT_STACK_SIZE);

    std::shared_ptr<FiberStack> get_fiber_stack();
private:
    size_t alloc_idx_;      // index for next allocation
    size_t stack_size_;     // size of each stack
    size_t stack_count_;    // number of stacks
    std::vector<std::shared_ptr<FiberStack>> stack_array_; // stacks that can be shared
};

// used to manage fiber environment per thread
class FiberEnvironment{
public:
    //init the env, init the main fiber
    FiberEnvironment();

    std::shared_ptr<Fiber> get_main_fiber() const { return fiber_call_stack_[0]; }
    std::shared_ptr<Fiber> get_current_fiber() const { return fiber_call_stack_.back(); }
    void push_fiber(std::shared_ptr<Fiber> fiber);
    void pop_fiber();
private:
    int call_stack_depth_;      // depth of the fiber call stack
    std::vector<std::shared_ptr<Fiber>> fiber_call_stack_;    // the call stack of fibers in this thread
    // // used for swapping stacks when using shared stacks
    // std::shared_ptr<Fiber> swap_in_fiber_;      // fiber to swap in
    // std::shared_ptr<Fiber> swap_out_fiber_;     // fiber to swap out
};

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::function<void*(void*)> fiber_func;
    //yeild
    //resume
    //poll

private:
    std::shared_ptr<FiberEnvironment> env_;
    FiberState state_;
    

    // 二选一
    std::shared_ptr<FiberStack> stack_;     //挂起时需保存并释放的栈空间
    const char* stack_buffer_top_;  // top of the saved stack buffer


    size_t stack_buffer_size_;
    std::unique_ptr<char[]> stack_buffer_;  // buffer the stack when the fiber is suspended

};










}


#endif