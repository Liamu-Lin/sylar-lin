#include "hook.h"

#include <dlfcn.h>

namespace sylar{
static thread_local bool t_hook_enable = false;

bool is_hook_enable(){
    return t_hook_enable;
}
void set_hook_enable(bool flag){
    t_hook_enable = flag;
}

}


#define OPERATE_ON_HOOK_FUNC(func) \
    func(sleep) \
    func(usleep)

#define DECLARE_HOOK_FUNC(name) \
    name##_fun name##_f = nullptr;
OPERATE_ON_HOOK_FUNC(DECLARE_HOOK_FUNC)
#undef DECLARE_HOOK_FUNC

struct _HookIniter{
    _HookIniter(){
        #define STORE_HOOK_FUNC(name) \
            name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
        OPERATE_ON_HOOK_FUNC(STORE_HOOK_FUNC)
        #undef STORE_HOOK_FUNC
    }
};
_HookIniter s_hook_initer;


extern "C"{

unsigned int sleep(unsigned int seconds){
    if(!sylar::is_hook_enable())
        return sleep_f(seconds);
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    if(!iom || fiber->is_main_fiber())
        return sleep_f(seconds);

    fiber->set_state(sylar::FiberState::SLEEPING);
    iom->add_timer(false, seconds * 1000, [iom, fiber](void*){
        fiber->set_state(sylar::FiberState::SUSPENDED);
        iom->add_fiber(fiber);
    });

    sylar::Fiber::fiber_yield();

    return 0;
}
int usleep(useconds_t usec){
    if(!sylar::is_hook_enable())
        return usleep_f(usec);
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    if(!iom || fiber->is_main_fiber())
        return usleep_f(usec);

    fiber->set_state(sylar::FiberState::SLEEPING);
    iom->add_timer(false, usec / 1000, [iom, fiber](void*){
        fiber->set_state(sylar::FiberState::SUSPENDED);
        iom->add_fiber(fiber);
    });

    sylar::Fiber::fiber_yield();

    return 0;
}



}