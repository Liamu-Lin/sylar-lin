#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include <memory>
#include <atomic>
#include <vector>
#include <list>

#include "log.h"
#include "fiber.h"
#include "thread.h"
#include "mutex.h"

namespace sylar{

class Scheduler : public std::enable_shared_from_this<Scheduler>{
public:
    typedef Mutex MutexType;
public:
    Scheduler(size_t thread_count = 1, const std::string& name = "default_schelduler_name");
    ~Scheduler();

    void start();
    void stop();

    void add_fiber(std::shared_ptr<Fiber> fiber, pid_t thread = -1);
    void add_fiber(sylar::fiber_func func, void* args = nullptr, pid_t thread = -1);

    bool has_idle_threads() const { return idle_thread_count_ > 0; }
    
    static Scheduler* GetThis();
protected:
    // TODO: change to std::function(...args)
    virtual void idle(void*);
    virtual void tickle();
    virtual bool can_stop();
private:
    // schedule fibers, run in each thread
    void schedule();
    void SetThis(Scheduler* scheduler);
private:
    struct ScheduledTask{
        pid_t thread_;
        std::shared_ptr<Fiber> fiber_;
    };
    enum State{ INIT, RUNNING, STOPPING, STOPPED };
private:
    mutable MutexType mutex_;
    std::string name_;
    size_t thread_count_;

    std::vector<std::shared_ptr<Thread>> threads_;
    std::list<ScheduledTask> tasks_;

    //State state_{INIT};
    std::atomic<State> state_{INIT};
    std::atomic<size_t> active_thread_count_{0};
    std::atomic<size_t> idle_thread_count_{0};
};


}


#endif