#include "fdmanager.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace sylar{

FdCtx::FdCtx(int fd):
    fd_(fd){
    recv_timeout_ = (uint64_t)-1;
    send_timeout_ = (uint64_t)-1;
    struct stat fd_stat;
    if(fstat(fd, &fd_stat) == 0){
        is_inited_ = true;
        is_closed_ = false;
        is_socket_ = S_ISSOCK(fd_stat.st_mode);
    }
    else{
        is_inited_ = false;
        is_socket_ = false;
        is_closed_ = true;
    }
    if(is_socket_ && !is_closed_){
        int flags = fcntl(fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK))
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    user_nonblock_ = false;
}

bool FdCtx::set_timeout(int type, uint64_t timeout){
    if(type == SO_RCVTIMEO)
        recv_timeout_ = timeout;
    else if(type == SO_SNDTIMEO)
        send_timeout_ = timeout;
    else
        return false;
    return true;
}
uint64_t FdCtx::get_timeout(int type) const{
    if(type == SO_RCVTIMEO)
        return recv_timeout_;
    else if(type == SO_SNDTIMEO)
        return send_timeout_;
    return -1;
}

FdManager::FdManager(){
}

std::shared_ptr<FdCtx> FdManager::get_fdctx(int fd, bool auto_create){
    std::shared_ptr<FdCtx> fd_ctx = nullptr;
    {
        RWMutex::RLock lock(fdctx_mutex_);
        auto it = fd_ctxs_.find(fd);
        if(it != fd_ctxs_.end())
            fd_ctx = it->second;
    }

    if(!auto_create || fd_ctx)
        return fd_ctx;

    fd_ctx.reset(new FdCtx(fd));
    RWMutex::WLock wlock(fdctx_mutex_);
    fd_ctxs_[fd] = fd_ctx;
    return fd_ctx;
}
bool FdManager::del_fdctx(int fd){
    RWMutex::WLock wlock(fdctx_mutex_);
    return fd_ctxs_.erase(fd) != 0;
}


}