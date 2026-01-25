#ifndef __SYLAR_HTTP_HTTP_SERVER_H__
#define __SYLAR_HTTP_HTTP_SERVER_H__

#include "tcp_server.h"
#include "http_session.h"
#include "http_servlet.h"
#include <memory>

namespace sylar{
namespace http{

class HttpServer: public sylar::TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;
public:
    HttpServer(IOManager* worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());
    virtual ~HttpServer();

    void set_keepalive(bool v) { is_keepalive_ = v; }
    bool is_keepalive() const { return is_keepalive_; }

    ServletDispatch::ptr get_servlet_dispatch() const { return servlet_dispatch_; }
protected:
    virtual void handle_client(Socket::ptr client, void*) override;
private:
    bool is_keepalive_;
    ServletDispatch::ptr servlet_dispatch_;
};

}
}

#endif