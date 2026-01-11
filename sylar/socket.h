#ifndef __SYLAR_SOCKET_H__
#define __SYLAR_SOCKET_H__

#include "log.h"
#include "address.h"
#include "noncopyable.h"

#include <memory>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace sylar{

class Socket : Noncopyable{
public:
    typedef std::shared_ptr<Socket> ptr;
    enum Type{ TCP = SOCK_STREAM, UDP = SOCK_DGRAM };
    enum Family{ IPv4 = AF_INET, IPv6 = AF_INET6, UNIX = AF_UNIX };
public:
    Socket(int family, int type, int protocol = 0);
    virtual ~Socket();

    bool is_valid() const;
    bool is_connected() const;
    int get_family() const;
    int get_type() const;
    int get_protocol() const;
    int get_fd() const;
    int get_error() const;
    uint64_t get_timeout(int type) const;
    Address::ptr get_local_address() const;
    Address::ptr get_remote_address() const;
    bool get_option(int level, int option, void* result, socklen_t* len) const;
    template<class T>
    bool get_option(int level, int option, T& result) const{
        socklen_t length = sizeof(T);
        return get_option(level, option, &result, &length);
    }

    void set_timeout(int type, uint64_t value);
    bool set_option(int level, int option, const void* result, socklen_t len);
    template<class T>
    bool set_option(int level, int option, const T& result){
        return set_option(level, option, &result, sizeof(T));
    }

    bool cancel_read();
    bool cancel_write();
    bool cancel_accept();
    bool cancel_all();

    virtual bool bind(const Address::ptr addr);
    virtual bool listen(int backlog = SOMAXCONN);
    virtual Socket::ptr accept();
    virtual bool connect(const Address::ptr addr);
    virtual bool reconnect(uint64_t timeout_ms = -1);
    virtual bool close();

    virtual int send(const void* buffer, size_t length, int flags = 0);
    virtual int send(const iovec* buffers, size_t length, int flags = 0);
    virtual int send_to(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    virtual int send_to(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    virtual int recv(void* buffer, size_t length, int flags = 0);
    virtual int recv(iovec* buffers, size_t length, int flags = 0);
    virtual int recv_from(void* buffer, size_t length, Address::ptr from, int flags = 0);
    virtual int recv_from(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const Socket& sock);
protected:
    virtual bool init(int sockfd);
protected:
    int sockfd_;
    int family_;
    int type_;
    int protocol_;
    bool is_connected_;
    mutable Address::ptr local_address_;
    mutable Address::ptr remote_address_;
};

}

#endif