#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include <memory>
#include <map>
#include <vector>
#include <atomic>
#include <sys/epoll.h>

#include "scheduler.h"
#include "fiber.h"
#include "mutex.h"

#define SYLAR_IOMANAGER_TIMEOUT 1000

namespace sylar{
class IOManager;
class IOEvent;

    // TODO: make it transparent to users
    class EventContext{
    private:
        void reset();
        void trigger();
        void set(Scheduler* scheduler, std::shared_ptr<Fiber> fiber);
    private:
        friend class IOEvent;
    private:
        Scheduler* scheduler_;
        std::shared_ptr<Fiber> fiber_;
    };

// TODO: make it transparent to users
class IOEvent{
public:
    IOEvent(int fd):fd_(fd){}
    uint32_t get_events() const;
private:
    bool add_event(uint32_t event, int epoll_fd, fiber_func func, void* args);
    bool del_event(uint32_t event, int epoll_fd);
    bool cancel_event(uint32_t event, int epoll_fd);
private:
    friend class IOManager;
private:
    int fd_ = -1;
    uint32_t events_ = 0x0;
    EventContext read_;
    EventContext write_;

    mutable Mutex mutex_;
};


class IOManager : public Scheduler{
public:
    IOManager(size_t thread_count = 1, const std::string& name = "default_iomanager_name");
    ~IOManager();

    bool add_event(int fd, uint32_t event, fiber_func func, void* args = nullptr);
    bool del_event(int fd, uint32_t event);
    bool cancel_event(int fd, uint32_t event);
    bool cancel_all(int fd);

    static IOManager* GetThis();
protected:
    void tickle() override;
    bool can_stop() override;
    void idle(void* args) override;
private:
    Semaphore sem_{0};
    int epoll_fd_ = -1;
    std::atomic<size_t> pending_event_count_{0};
    std::map<int, std::shared_ptr<IOEvent>> fd_event_map_;

    mutable RWMutex rw_mutex_;
};


}


#endif