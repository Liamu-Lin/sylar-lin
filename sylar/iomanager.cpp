#include "iomanager.h"


namespace sylar{


void EventContext::reset(){
    scheduler_.reset();
    fiber_.reset();
}
void EventContext::trigger(){
    if(!scheduler_ || !fiber_)
        return;
    scheduler_->add_fiber(fiber_);
    reset();
}
void EventContext::set(std::shared_ptr<Scheduler> scheduler, std::shared_ptr<Fiber> fiber){
    scheduler_ = scheduler;
    fiber_ = fiber;
}



uint32_t IOEvent::get_events() const{
    Mutex::Lock lock(mutex_);
    return events_;
}
bool IOEvent::add_event(uint32_t event, int epoll_fd){
    Mutex::Lock lock(mutex_);
    uint32_t old_events = events_;
    int op = events_ ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

    events_ |= event;
    if(event & EPOLLIN)
        read_.set(Scheduler::GetThis(), Fiber::get_this());
    if(event & EPOLLOUT)
        write_.set(Scheduler::GetThis(), Fiber::get_this());

    epoll_event ep_event;
    ep_event.events = EPOLLET | events_;
    ep_event.data.ptr = this;
    int rt = epoll_ctl(epoll_fd, op, fd_, &ep_event);
    if(rt){
        SYLAR_LOG(LoggerMgr.get_logger("system"), LogLevel::Level::ERROR)
            << "epoll_ctl add/mod error rt=" << rt
            << " fd=" << fd_ << " event=" << event;
        events_ = old_events;
        return false;
    }
    return true;
}
bool IOEvent::del_event(uint32_t event, int epoll_fd){
    Mutex::Lock lock(mutex_);
    if((events_ & event) == 0)
        return false;

    events_ &= ~event;
    if(event & EPOLLIN)
        read_.reset();
    if(event & EPOLLOUT)
        write_.reset();

    int op = events_ ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event;
    ep_event.events = EPOLLET | events_;
    ep_event.data.ptr = this;
    int rt = epoll_ctl(epoll_fd, op, fd_, &ep_event);
    if(rt){
        SYLAR_LOG(LoggerMgr.get_logger("system"), LogLevel::Level::ERROR)
            << "epoll_ctl del/mod error rt=" << rt
            << " fd=" << fd_ << " event=" << event;
        return false;
    }
    return true;
}
bool IOEvent::cancel_event(uint32_t event, int epoll_fd){
    {
        Mutex::Lock lock(mutex_);
        if((events_ & event) == 0)
            return false;

        if(event & EPOLLIN)
            read_.trigger();
        if(event & EPOLLOUT)
            write_.trigger();
    }
    return del_event(event, epoll_fd);
}


IOManager::IOManager(size_t thread_count, const std::string& name):
    Scheduler(thread_count, name){
    epoll_fd_ = epoll_create(1024);
    SYLAR_ASSERT(epoll_fd_ > 0);

    start();
}
IOManager::~IOManager(){
    stop();
    close(epoll_fd_);
}

bool IOManager::add_event(int fd, uint32_t event, fiber_func func, void* args){
    bool ret;
    std::shared_ptr<IOEvent> io_event;
    {
        RWMutex::RLock lock(rw_mutex_);
        auto it = fd_event_map_.find(fd);
        if(it != fd_event_map_.end())
            io_event = it->second;
    }
    if(io_event){
        RWMutex::WLock lock(rw_mutex_);
        ret = io_event->add_event(event, epoll_fd_);
    }
    else{
        io_event.reset(new IOEvent);
        ret = io_event->add_event(event, epoll_fd_);
        RWMutex::WLock lock(rw_mutex_);
        fd_event_map_[fd] = io_event;
    }

    if(ret == false)
        return false;
    pending_event_count_ += 1;
    return true;
}

bool IOManager::del_event(int fd, uint32_t event){
    bool ret = false;
    std::shared_ptr<IOEvent> io_event;
    {
        RWMutex::RLock lock(rw_mutex_);
        auto it = fd_event_map_.find(fd);
        if(it != fd_event_map_.end())
            io_event = it->second;
    }
    if(io_event){
        RWMutex::WLock lock(rw_mutex_);
        ret = io_event->del_event(event, epoll_fd_);
    }

    if(ret == false)
        return false;
    pending_event_count_ -= 1;
    return true;
}

bool IOManager::cancel_event(int fd, uint32_t event){
    bool ret = false;
    std::shared_ptr<IOEvent> io_event;
    {
        RWMutex::RLock lock(rw_mutex_);
        auto it = fd_event_map_.find(fd);
        if(it != fd_event_map_.end())
            io_event = it->second;
    }
    if(io_event){
        RWMutex::WLock lock(rw_mutex_);
        ret = io_event->cancel_event(event, epoll_fd_);
    }

    if(ret == false)
        return false;
    pending_event_count_ -= 1;

    return true;
}

bool IOManager::cancel_all(int fd){
    bool ret = false;
    std::shared_ptr<IOEvent> io_event;
    {
        RWMutex::RLock lock(rw_mutex_);
        auto it = fd_event_map_.find(fd);
        if(it != fd_event_map_.end())
            io_event = it->second;
    }
    if(io_event){
        RWMutex::WLock lock(rw_mutex_);
        uint32_t events = io_event->get_events();
        if(events & EPOLLIN){
            bool rt = io_event->cancel_event(EPOLLIN, epoll_fd_);
            if(rt) pending_event_count_ -= 1;
            ret |= rt;
        }
        if(events & EPOLLOUT){
            bool rt = io_event->cancel_event(EPOLLOUT, epoll_fd_);
            if(rt) pending_event_count_ -= 1;
            ret |= rt;
        }
    }
    return ret;
}

std::shared_ptr<IOManager> IOManager::GetThis(){
    return std::dynamic_pointer_cast<IOManager>(Scheduler::GetThis());
}

void IOManager::tickle(){
    if(has_idle_threads())
        sem_.Post();
}

bool IOManager::can_stop(){
    return Scheduler::can_stop() && pending_event_count_ == 0;
}

void IOManager::idle(void* args){
    epoll_event* events = new epoll_event[64]();
    while(true){
        if(can_stop())
            break;

        int rt = 0;
        while(true){
            rt = epoll_wait(epoll_fd_, events, 64, SYLAR_IOMANAGER_TIMEOUT);
            if(rt != 0 || errno != EINTR)
                break;
        }
        // process events
        for(int i = 0; i < rt; ++i){
            uint32_t revents = events[i].events;
            IOEvent* io_event = (IOEvent*)events[i].data.ptr;
            if(revents & (EPOLLERR | EPOLLHUP))
                revents |= (EPOLLIN | EPOLLOUT);
            
            if(revents & EPOLLIN){
                io_event->cancel_event(EPOLLIN, epoll_fd_);
                pending_event_count_ -= 1;
            }
            if(revents & EPOLLOUT){
                io_event->cancel_event(EPOLLOUT, epoll_fd_);
                pending_event_count_ -= 1;
            }
        }
        if(rt > 0){
            delete[] events;
            Fiber::fiber_yield();
            events = new epoll_event[64]();
        }

    }
    delete[] events;
}



}