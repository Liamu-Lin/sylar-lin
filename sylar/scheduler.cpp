#include "scheduler.h"
#include "macro.h"

namespace sylar{

static thread_local std::shared_ptr<Scheduler> g_scheduler;

std::shared_ptr<Scheduler> Scheduler::GetThis(){
    return g_scheduler;
}
void Scheduler::SetThis(){
    g_scheduler = shared_from_this();
}

Scheduler::Scheduler(size_t thread_count, const std::string& name):
    name_(name),
    thread_count_(thread_count){
    ;
}
Scheduler::~Scheduler(){
    ;
}

void Scheduler::add_fiber(std::shared_ptr<Fiber> fiber, pid_t thread){
    ScheduledTask task = {thread, fiber};
    MutexType::Lock lock(mutex_);
    tasks_.push_back(task);
}
void Scheduler::add_fiber(sylar::fiber_func func, void* args, pid_t thread){
    std::shared_ptr<Fiber> fiber(new Fiber(func, args));
    ScheduledTask task = {thread, fiber};
    MutexType::Lock lock(mutex_);
    tasks_.push_back(task);
}

void Scheduler::start(){
    MutexType::Lock lock(mutex_);
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
    if(state_.load() != RUNNING)
        return;
    SYLAR_ASSERT(GetThis().get() != this);

    state_.store(STOPPING);
    // tickle all threads to wake them up
    for(size_t i = 0; i < thread_count_; ++i)
        tickle();
    
    std::vector<std::shared_ptr<Thread>> thrs;
    {
        MutexType::Lock lock(mutex_);
        thrs.swap(threads_);
    }
    for(auto& thr : thrs)
        thr->join();
}

void Scheduler::schedule(){
    SetThis();

    std::shared_ptr<Fiber> idle_fiber(new Fiber(std::bind(&Scheduler::idle
                                      , this, std::placeholders::_1), nullptr));
    
    while(true){
        // // if the scheduler is stopping, stop scheduling
        // if(state_.load() == STOPPING)
        //     break;
        bool have_task = false;
        bool tickle_others = false;
        //chose an available task
        ScheduledTask task;
        {
            MutexType::Lock lock(mutex_);
            auto it = tasks_.begin();
            while(it != tasks_.end()){
                if(it->thread_ != -1 && it->thread_ != sylar::Thread::get_id()){
                    tickle_others = true;
                    ++it;
                    continue;
                }
                task = *it;
                have_task = true;
                it = tasks_.erase(it);
                break;
            }
            if(it != tasks_.end())
                tickle_others = true;
        }

        if(tickle_others)
            tickle();

        if(have_task && task.fiber_->get_state() != FiberState::TERMINATED
                     && task.fiber_->get_state() != FiberState::EXCEPTION){

            active_thread_count_.fetch_add(1);
            task.fiber_->set_env();
            task.fiber_->fiber_resume();
            active_thread_count_.fetch_sub(1);
            if(task.fiber_->get_state() == FiberState::READY ||
               task.fiber_->get_state() == FiberState::SUSPENDED){
                add_fiber(task.fiber_);
            }
        }
        // no task, run idle fiber
        else{
            // scheduler is stopping
            if(idle_fiber->get_state() == FiberState::TERMINATED
               || idle_fiber->get_state() == FiberState::EXCEPTION){
                SYLAR_LOG(LoggerMgr.get_logger("system"), LogLevel::Level::DEBUG)
                    << "Idle Fiber Terminated or Exception";
                break;
            }
            idle_thread_count_.fetch_add(1);
            idle_fiber->fiber_resume();
            idle_thread_count_.fetch_sub(1);
        }

    }

}

void Scheduler::idle(void*){
    SYLAR_LOG(LoggerMgr.get_logger("system"), LogLevel::Level::DEBUG)
        << "Scheduler::idle";
    while(!can_stop()){
        sylar::Fiber::fiber_yield();
    }
}

void Scheduler::tickle(){
    SYLAR_LOG(LoggerMgr.get_logger("system"), LogLevel::Level::DEBUG)
        << "Scheduler::tickle";
}

bool Scheduler::can_stop(){
    MutexType::Lock lock(mutex_);
    return state_ == STOPPING
           && threads_.empty()
           && active_thread_count_ == 0
           && tasks_.empty();
}

}