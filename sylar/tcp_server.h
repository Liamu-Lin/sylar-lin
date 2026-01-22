#ifndef __SYLAR_TCP_SERVER_H__
#define __SYLAR_TCP_SERVER_H__

#include "log.h"
#include "iomanager.h"
#include "socket.h"
#include "address.h"
#include "noncopyable.h"

#include <vector>
#include <string>

namespace sylar{

class TcpServer: Noncopyable, public std::enable_shared_from_this<TcpServer>{
public:
    typedef std::shared_ptr<TcpServer> ptr;
public:
    TcpServer(IOManager* worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());
    virtual ~TcpServer();

    const std::string& get_name() const { return name_; }
    void set_name(const std::string& v) { name_ = v; }

    uint64_t get_recv_timeout() const { return recv_timeout_; }
    void set_recv_timeout(uint64_t v) { recv_timeout_ = v; }

    bool is_stop() const { return is_stop_; }

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>* fails = nullptr);

    virtual bool start();
    virtual bool stop();

    friend std::ostream& operator<<(std::ostream& os, const TcpServer& server);
protected:
    virtual void on_accepted(Socket::ptr sock, void*);
    virtual void handle_client(Socket::ptr client, void*);
protected:
    std::string name_;
    IOManager* worker_;
    IOManager* accept_worker_;
    uint64_t recv_timeout_;
    bool is_stop_;
    std::vector<Socket::ptr> listen_socks_;
};



}

#endif