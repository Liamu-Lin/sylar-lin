#include "hook.h"
#include "config.h"
#include "fdmanager.h"

#include <dlfcn.h>
#include <fcntl.h>

namespace sylar{
static thread_local bool t_hook_enable = false;
static ConfigVar<uint64_t>::ptr g_connect_timeout =
    ConfigMgr.look_up<uint64_t>("tcp.connect.timeout_ms", 5000, "connect timeout in ms");
static uint64_t t_connect_timeout = -1;

bool is_hook_enable(){
    return t_hook_enable;
}
void set_hook_enable(bool flag){
    t_hook_enable = flag;
}
}

#define OPERATE_ON_HOOK_FUNC(func) \
    func(sleep) \
    func(usleep) \
    func(nanosleep) \
    func(socket) \
    func(connect) \
    func(accept) \
    func(read) \
    func(readv) \
    func(recv) \
    func(recvfrom) \
    func(recvmsg) \
    func(write) \
    func(writev) \
    func(send) \
    func(sendto) \
    func(sendmsg) \
    func(close) \
    func(fcntl) \
    func(ioctl) \
    func(getsockopt) \
    func(setsockopt)


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

        sylar::t_connect_timeout = sylar::g_connect_timeout->get_value();
        sylar::g_connect_timeout->add_listener([](const uint64_t& old_value, const uint64_t& new_value){
            sylar::t_connect_timeout = new_value;
        });
    }
};
_HookIniter s_hook_initer;

struct timer_info{
    int state = 0;
};
template<typename SysFunc, typename... Args>
ssize_t hook_iofunc(const char* fun_name, SysFunc sys_fun, int sockfd, 
                    uint32_t event, int timeout_type, Args&&... args){
    if(!sylar::is_hook_enable())
        return sys_fun(sockfd, std::forward<Args>(args)...);
    sylar::FdManager& fd_mgr = sylar::FdManager::Instance();
    std::shared_ptr<sylar::FdCtx> fd_ctx = fd_mgr.get_fdctx(sockfd);
    if(!fd_ctx || fd_ctx->is_closed() || !fd_ctx->is_socket() || fd_ctx->get_user_nonblock())
        return sys_fun(sockfd, std::forward<Args>(args)...);

RETRY:
    // try to do it directly
    ssize_t rt = sys_fun(sockfd, std::forward<Args>(args)...);
    // interrupted, retry
    while(rt == -1 && errno == EINTR)
        rt = sys_fun(sockfd, std::forward<Args>(args)...);
    // failed and wait the event to be ready
    if(rt == -1 && errno == EAGAIN){
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);
        sylar::IOManager* iom = sylar::IOManager::GetThis();
        sylar::Timer::ptr timer;
        std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
        SYLAR_ASSERT(iom && !fiber->is_main_fiber());

        // add timer if timeout is set
        uint64_t timeout = fd_ctx->get_timeout(timeout_type);
        if(timeout != (uint64_t)-1){
            timer = iom->add_timer(false, timeout, [sockfd, event, iom, winfo, fiber](void*){
                auto t = winfo.lock();
                t->state = ETIMEDOUT;
                fiber->fiber_wakeup();
                iom->cancel_event(sockfd, event);
            }, winfo, nullptr);
        }
        // add io event
        bool ret = iom->add_event(sockfd, event, fiber);
        if(ret == false){
            if(timer) timer->cancel();
            return -1;
        }
        else{
            fiber->fiber_sleep();
            if(timer) timer->cancel();
            if(tinfo->state == ETIMEDOUT){
                errno = ETIMEDOUT;
                return -1;
            }
            goto RETRY;
        }
    }
    return rt;
}
extern "C"{
// sleep
unsigned int sleep(unsigned int seconds){
    if(!sylar::is_hook_enable())
        return sleep_f(seconds);
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    if(!iom || fiber->is_main_fiber())
        return sleep_f(seconds);

    iom->add_timer(false, seconds * 1000, [iom, fiber](void*){
        fiber->fiber_wakeup();
        iom->add_fiber(fiber);
    });
    sylar::Fiber::fiber_sleep();

    return 0;
}
int usleep(useconds_t usec){
    if(!sylar::is_hook_enable())
        return usleep_f(usec);
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    if(!iom || fiber->is_main_fiber())
        return usleep_f(usec);

    iom->add_timer(false, usec / 1000, [iom, fiber](void*){
        fiber->fiber_wakeup();
        iom->add_fiber(fiber);
    });
    sylar::Fiber::fiber_sleep();

    return 0;
}
int nanosleep(const struct timespec *req, struct timespec *rem){
    if(!sylar::is_hook_enable())
        return nanosleep_f(req, rem);
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    if(!iom || fiber->is_main_fiber())
        return nanosleep_f(req, rem);

    uint64_t time_ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;

    iom->add_timer(false, time_ms, [iom, fiber](void*){
        fiber->fiber_wakeup();
        iom->add_fiber(fiber);
    });
    sylar::Fiber::fiber_sleep();
    return 0;
}

// socket
int socket(int domain, int type, int protocol){
    if(!sylar::is_hook_enable())
        return socket_f(domain, type, protocol);
    int fd = socket_f(domain, type, protocol);
    if(fd != -1){
        // create fd context, and make it non-block
        sylar::FdManager::Instance().get_fdctx(fd, true);
    }
    return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    if(!sylar::is_hook_enable())
        return connect_f(sockfd, addr, addrlen);
    sylar::FdManager& fd_mgr = sylar::FdManager::Instance();
    std::shared_ptr<sylar::FdCtx> fd_ctx = fd_mgr.get_fdctx(sockfd);
    if(!fd_ctx || fd_ctx->is_closed()){
        errno = EBADF;
        return -1;
    }
    else if(!fd_ctx->is_socket() || fd_ctx->get_user_nonblock())
        return connect_f(sockfd, addr, addrlen);
    // connect and wait for completion
    int rt = connect_f(sockfd, addr, addrlen);
    if(rt == 0 || errno != EINPROGRESS){
        // connect success immediately, or error happens
        return rt;
    }

    sylar::IOManager* iom = sylar::IOManager::GetThis();
    std::shared_ptr<sylar::Fiber> fiber = sylar::Fiber::get_this();
    SYLAR_ASSERT(iom && !fiber->is_main_fiber());
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);
    auto timer = iom->add_timer(false, sylar::t_connect_timeout, [fiber, winfo, iom, sockfd](void*){
                        fiber->fiber_wakeup();
                        winfo.lock()->state = ETIMEDOUT;
                        iom->cancel_event(sockfd, EPOLLOUT);
                    }, winfo, nullptr);
    bool ret = iom->add_event(sockfd, EPOLLOUT, fiber);
    if(ret == false){
        // add event failed
        if(timer)   timer->cancel();
        return -1;
    }
    else{
        fiber->fiber_sleep();
        if(timer)
            timer->cancel();
        if(tinfo->state == ETIMEDOUT){
            errno = ETIMEDOUT;
            return -1;
        }
    }
    // check the socket error
    int error = 0;
    socklen_t len = sizeof(error);
    if(-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len))
        return -1;
    if(error){
        errno = error;
        return -1;
    }
    return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    ssize_t rt = hook_iofunc("accept", accept_f, sockfd, EPOLLIN, SO_RCVTIMEO, addr, addrlen);
    int fd = (int)rt;
    if(fd != -1)
        sylar::FdManager::Instance().get_fdctx(fd, true);
    return fd;
}

int close(int fd){
    if(!sylar::is_hook_enable())
        return close_f(fd);
    std::shared_ptr<sylar::FdCtx> fd_ctx = sylar::FdMgr.get_fdctx(fd);
    if(fd_ctx){
        sylar::IOManager* iom = sylar::IOManager::GetThis();
        if(iom){
            iom->cancel_all(fd);
        }
        sylar::FdMgr.del_fdctx(fd);
    }
    return close_f(fd);
}

// read
ssize_t read(int fd, void *buf, size_t count) {
    return hook_iofunc("read", read_f, fd, EPOLLIN, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return hook_iofunc("readv", readv_f, fd, EPOLLIN, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return hook_iofunc("recv", recv_f, sockfd, EPOLLIN, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return hook_iofunc("recvfrom", recvfrom_f, sockfd, EPOLLIN, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return hook_iofunc("recvmsg", recvmsg_f, sockfd, EPOLLIN, SO_RCVTIMEO, msg, flags);
}

// write
ssize_t write(int fd, const void *buf, size_t count) {
    return hook_iofunc("write", write_f, fd, EPOLLOUT, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return hook_iofunc("writev", writev_f, fd, EPOLLOUT, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return hook_iofunc("send", send_f, s, EPOLLOUT, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return hook_iofunc("sendto", sendto_f, s, EPOLLOUT, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return hook_iofunc("sendmsg", sendmsg_f, s, EPOLLOUT, SO_SNDTIMEO, msg, flags);
}

// fdcntl
int fcntl(int fd, int cmd, ... /* arg */ ){
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                std::shared_ptr<sylar::FdCtx> ctx = sylar::FdMgr.get_fdctx(fd);
                if(!ctx || ctx->is_closed() || !ctx->is_socket())
                    return fcntl_f(fd, cmd, arg);
                ctx->set_user_nonblock(arg & O_NONBLOCK);
                arg |= O_NONBLOCK;
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                std::shared_ptr<sylar::FdCtx> ctx = sylar::FdMgr.get_fdctx(fd);
                if(!ctx || ctx->is_closed() || !ctx->is_socket()) {
                    return arg;
                }
                if(ctx->get_user_nonblock())
                    return arg | O_NONBLOCK;
                else
                    return arg & ~O_NONBLOCK;
            }
            break;
        case F_DUPFD: case F_DUPFD_CLOEXEC: case F_SETFD:
        case F_SETOWN: case F_SETSIG: case F_SETLEASE: case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD: case F_GETOWN: case F_GETSIG: case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK: case F_SETLKW: case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX: case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(request == FIONBIO) {
        bool user_nonblock = !!*(int*)arg;
        std::shared_ptr<sylar::FdCtx> ctx = sylar::FdMgr.get_fdctx(d);
        if(!ctx || ctx->is_closed() || !ctx->is_socket())
            return ioctl_f(d, request, arg);
        ctx->set_user_nonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!sylar::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            std::shared_ptr<sylar::FdCtx> ctx = sylar::FdMgr.get_fdctx(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->set_timeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}