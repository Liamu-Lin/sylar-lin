#include "timer.h"


namespace sylar{

Timer::Timer(bool recurring, uint64_t ms, fiber_func cb, TimerManager* manager):
    is_recurring_(recurring),
    period_time_(ms),
    cb_(cb),
    manager_(manager){
    next_expiration_ = sylar::TimeUtil::get_time_ms() + period_time_;
}

bool Timer::cancel(){
    return manager_->del_timer(shared_from_this());
}

bool Timer::refresh(){
    bool ret = false;
    std::shared_ptr<Timer> self = shared_from_this();
    ret = manager_->del_timer(self);
    if(ret){
        next_expiration_ = sylar::TimeUtil::get_time_ms() + period_time_;
        ret = manager_->add_timer(is_recurring_, period_time_, cb_);
    }
    return ret;
}

bool Timer::reset(uint64_t ms, bool from_now){
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
            ret = manager_->add_timer(is_recurring_, period_time_, cb_);
        }
    }
    return ret;
}

}