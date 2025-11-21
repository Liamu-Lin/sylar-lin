#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <ucontext.h>

#include <memory>

namespace sylar{

enum class FiberState : int{
    INIT,
    READY,
    EXEC,
    HOLD,
    TERM,
    EXCEPT
};

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;

    //yeild
    //resume
    //poll

private:
    ucontext_t ctx_;
    ucontext_t caller_ctx_; // context to switch back to caller
    FiberState state_;
};










}


#endif