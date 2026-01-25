#ifndef __SYLAR_HTTP_HTTP_SESSION_H__
#define __SYLAR_HTTP_HTTP_SESSION_H__

#include "http/http11_parser.h"
#include "stream.h"
#include "http/http.h"

namespace sylar{
namespace http{

class HttpSession: public SocketStream{
public:
    typedef std::shared_ptr<HttpSession> ptr;
public:
    HttpSession(Socket::ptr sock);
    ~HttpSession() override = default;
    HttpRequest::ptr recv_request();
    void send_response(HttpResponse::ptr rsp);
};

}
}

#endif