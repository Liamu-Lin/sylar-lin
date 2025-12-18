#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include <memory>
#include <stdint.h>

#include "fiber.h"

namespace sylar{

class TimerManager;

class Timer: public std::enable_shared_from_this<Timer> {
    friend class TimerManager;
public:
    bool cancel();
    // restart the timer
    bool refresh();
    // reset the timer with new time(ms)
    // if from_now is true, the time is counted from now;
    // otherwise, from the initial/last expiration time
    bool reset(uint64_t ms, bool from_now);
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
    Timer(bool recurring, uint64_t ms, fiber_func cb, TimerManager* manager);
private:
    bool is_recurring_;
    uint64_t period_time_;
    uint64_t next_expiration_;
    fiber_func cb_;
    TimerManager* manager_;
};

class TimerManager{
public:
    bool add_timer(bool recurring, uint64_t ms, fiber_func cb);
    bool del_timer(std::shared_ptr<Timer> timer);
};

}

#endif