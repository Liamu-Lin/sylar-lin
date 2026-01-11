#ifndef __SYLAR_FDMANAGER_H__
#define __SYLAR_FDMANAGER_H__

#include "mutex.h"
#include "singleton.h"

#include <map>
#include <memory>

#include <stdint.h>

namespace sylar{

class FdCtx{
public:
    typedef std::shared_ptr<FdCtx> ptr;
public:
    FdCtx(int fd);

    bool is_inited() const { return is_inited_; }
    bool is_socket() const { return is_socket_; }
    bool is_closed() const { return is_closed_; }

    void set_user_nonblock(bool v) { user_nonblock_ = v; }
    bool get_user_nonblock() const { return user_nonblock_; }

    bool set_timeout(int type, uint64_t timeout);
    uint64_t get_timeout(int type) const;
private:
    bool is_inited_: 1;
    bool is_socket_: 1;
    bool is_closed_: 1;
    bool user_nonblock_: 1; // user implement nonblock machanism, instead of system
    int fd_;
    uint64_t recv_timeout_;
    uint64_t send_timeout_;
};

class FdManager: public Singleton<FdManager>{
Singleton_Constructor(FdManager)
public:
    std::shared_ptr<FdCtx> get_fdctx(int fd, bool auto_create = false);
    bool del_fdctx(int fd);
private:
    RWMutex fdctx_mutex_;
    std::map<int, std::shared_ptr<FdCtx>> fd_ctxs_;
};
#define FdMgr FdManager::Instance()


}

#endif