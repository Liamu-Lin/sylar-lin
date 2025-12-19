#include "timer.h"


namespace sylar{

Timer::Timer(bool recurring, uint64_t ms, fiber_func cb, void* args, TimerManager* manager):
    is_recurring_(recurring),
    period_time_(ms),
    cb_(cb),
    args_(args),
    manager_(manager){
    next_expiration_ = sylar::TimeUtil::get_time_ms() + period_time_;
}

void Timer::run(){
    cb_(args_);
}

bool Timer::cancel(){
    bool ret = false;
    if(manager_){
        ret = manager_->del_timer(shared_from_this());
        manager_ = nullptr;
    }
    return ret;
}

bool Timer::refresh(){
    if(!manager_)
        return false;
    bool ret = false;
    std::shared_ptr<Timer> self = shared_from_this();
    ret = manager_->del_timer(self);
    if(ret){
        next_expiration_ = sylar::TimeUtil::get_time_ms() + period_time_;
        ret = manager_->add_timer(self);
    }
    return ret;
}

bool Timer::reset(uint64_t ms, bool from_now){
    if(!manager_)
        return false;
    bool ret = false;
    uint64_t now = 0;
    if(from_now)
        now = sylar::TimeUtil::get_time_ms();
    else
        now = next_expiration_ - period_time_;
    period_time_ = ms;
    uint64_t new_expiration = now + period_time_;
    
    if(new_expiration != next_expiration_){
        std::shared_ptr<Timer> self = shared_from_this();
        ret = manager_->del_timer(self);
        if(ret){
            next_expiration_ = new_expiration;
            ret = manager_->add_timer(self);
        }
    }
    return ret;
}

Timer::ptr TimerManager::add_timer(bool recurring, uint64_t ms, fiber_func cb, void* args){
    std::shared_ptr<Timer> timer(new Timer(recurring, ms, cb, args, this));
    if(add_timer(timer))
        return timer;
    return nullptr;
}
Timer::ptr TimerManager::add_timer(bool recurring, uint64_t ms, 
                             fiber_func cb, std::weak_ptr<void> cond, void* args){
    std::shared_ptr<Timer> timer(new Timer(recurring, ms,
            std::bind(cond_timer_cb, cb, cond, std::placeholders::_1), args, this));
    if(add_timer(timer))
        return timer;
    return nullptr;
}
bool TimerManager::add_timer(Timer::ptr timer){
    if(!timer)
        return false;
    bool ret = false;
    bool is_front = false;
    {
        RWMutex::WLock lock(timers_rwmutex_);
        auto it = timers_.insert(timer);
        ret = it.second;
        is_front = (it.first == timers_.begin() && need_tickle_);
    }
    if(is_front){
        need_tickle_ = false;
        on_timer_insert_at_front();
    }
    return ret;
}
bool TimerManager::del_timer(Timer::ptr timer){
    RWMutex::WLock lock(timers_rwmutex_);
    auto it = timers_.find(timer);
    if(it != timers_.end()){
        timers_.erase(it);
        return true;
    }
    return false;
}

uint64_t TimerManager::get_next_timer() const{
    RWMutex::RLock lock(timers_rwmutex_);
    if(timers_.empty())
        return ~0ull;
    need_tickle_ = true;
    const std::shared_ptr<Timer>& timer = *timers_.begin();
    uint64_t now = sylar::TimeUtil::get_time_ms();
    if(now >= timer->next_expiration_)
        return 0;
    else
        return timer->next_expiration_ - now;
}

void TimerManager::get_expired_timers(std::vector<Timer::ptr> expired_timers){
    RWMutex::WLock lock(timers_rwmutex_);
    uint64_t now_time = TimeUtil::get_time_ms();
    Timer::ptr now_timer(new Timer(now_time));
    auto end = timers_.upper_bound(now_timer);

    if(end == timers_.end())
        return;
    need_tickle_ = true;
    expired_timers = std::vector<Timer::ptr>(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);
    for(auto it : expired_timers){
        if(it->is_recurring_){
            it->next_expiration_ = now_time + it->period_time_;
            timers_.insert(it);
        }
    }
}

bool TimerManager::empty() const{
    RWMutex::RLock lock(timers_rwmutex_);
    return timers_.empty();
}

void TimerManager::cond_timer_cb(fiber_func cb, std::weak_ptr<void> cond, void* args){
    std::shared_ptr<void> locker = cond.lock();
    if(locker)
        cb(args);
}


}