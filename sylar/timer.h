#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include <memory>
#include <atomic>
#include <vector>
#include <set>
#include <stdint.h>

#include "fiber.h"
#include "mutex.h"

namespace sylar{

class TimerManager;

class Timer: public std::enable_shared_from_this<Timer> {
public:
    friend class TimerManager;
    typedef std::shared_ptr<Timer> ptr;
public:
    bool cancel();
    // restart the timer
    bool refresh();
    // reset the timer with new time(ms)
    // if from_now is true, the time is counted from now;
    // otherwise, from the initial/last expiration time
    bool reset(uint64_t ms, bool from_now);
    void* get_args() const { return args_; }
    fiber_func get_callback() const { return cb_; }
public:
    class Comparator{
    public:
        bool operator()(const std::shared_ptr<Timer>& lhs, 
                        const std::shared_ptr<Timer>& rhs) const {
            if(!lhs || !rhs)
                return lhs == nullptr;
            return lhs->next_expiration_ < rhs->next_expiration_;
        }
    };
private:
    Timer(bool recurring, uint64_t ms, fiber_func cb, void* args_, TimerManager* manager);
    Timer(u_int64_t ms):next_expiration_(ms){}
private:
    bool is_recurring_;
    uint64_t period_time_;
    uint64_t next_expiration_;
    fiber_func cb_;
    void* args_;
    TimerManager* manager_;
};

class TimerManager{
    friend class Timer;
public:
    Timer::ptr add_timer(bool recurring, uint64_t ms, fiber_func cb, void* args = nullptr);
    Timer::ptr add_timer(bool recurring, uint64_t ms, fiber_func cb, std::weak_ptr<void> cond, void* args = nullptr);
    bool del_timer(Timer::ptr timer);
    uint64_t get_next_timer() const;
    void get_expired_timers(std::vector<Timer::ptr>& expired_timers);
    bool empty() const;
private:
    bool add_timer(Timer::ptr timer);
    static void cond_timer_cb(fiber_func cb, std::weak_ptr<void> cond, void* args);
protected:
    virtual void on_timer_insert_at_front() = 0;
private:
    mutable RWMutex timers_rwmutex_;
    mutable std::atomic<bool> need_tickle_{false};
    std::set<Timer::ptr, Timer::Comparator> timers_;
};

}

#endif