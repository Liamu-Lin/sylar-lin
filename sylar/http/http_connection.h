#ifndef __SYLAR_HTTP_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_HTTP_CONNECTION_H__

#include "socket.h"
#include "stream.h"
#include "http.h"

namespace sylar{
namespace http{

class HttpConnection: public SocketStream{
public:
    typedef std::shared_ptr<HttpConnection> ptr;
public:
    HttpConnection(Socket::ptr sock);
    ~HttpConnection() override = default;

    HttpResponse::ptr recv_response();
    void send_request(HttpRequest::ptr req);
private:
};

}
}

#endif