#include "scheduler.h"
#include "macro.h"

namespace sylar{

static std::shared_ptr<Scheduler> g_scheduler;

std::shared_ptr<Scheduler> Scheduler::GetThis(){
    return g_scheduler;
}
void Scheduler::SetThis(){
    g_scheduler = shared_from_this();
}

Scheduler::Scheduler(size_t thread_count, const std::string& name):
    name_(name_),
    thread_count_(thread_count){
    ;
}
Scheduler::~Scheduler(){
    ;
}

void Scheduler::add_fiber(std::shared_ptr<Fiber> fiber){
    ScheduledTask task = {-1, fiber};
    Mutex::Lock lock(mutex_);
    tasks_.push_back(task);
}
void Scheduler::add_fiber(sylar::fiber_func func, void* args){
    std::shared_ptr<Fiber> fiber(new Fiber(func, args));
    ScheduledTask task = {-1, fiber};
    Mutex::Lock lock(mutex_);
    tasks_.push_back(task);
}

void Scheduler::start(){
    Mutex::Lock lock(mutex_);
    if(state_.load() != INIT)
        return;
    state_.store(RUNNING);

    threads_.resize(thread_count_);
    for(size_t i = 0; i < thread_count_; ++i){
        threads_[i].reset(new Thread(std::bind(&Scheduler::schedule, this)
                          , name_ + "_" + std::to_string(i)));
    }
}
void Scheduler::stop(){
    // forbidden stop the scheduler in the thread scheduled by itself
    if(state_.load() != RUNNING || GetThis().get() == this)
        return;
    state_.store(STOPPING);
    // tickle all threads to wake them up
    for(size_t i = 0; i < thread_count_; ++i)
        tickle();
    
    std::vector<std::shared_ptr<Thread>> thrs;
    {
        Mutex::Lock lock(mutex_);
        thrs.swap(threads_);
    }
    for(auto& thr : thrs)
        thr->join();
}

void Scheduler::schedule(){
    SetThis();

    std::shared_ptr<Fiber> idle_fiber(new Fiber(fiber_func(std::bind(&Scheduler::idle, this)), nullptr));

}

}