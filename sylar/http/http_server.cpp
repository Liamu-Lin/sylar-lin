#include "http/http_server.h"
#include "log.h"

namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

HttpServer::HttpServer(IOManager* worker, IOManager* accept_worker)
    :TcpServer(worker, accept_worker),
    is_keepalive_(false){
    ;
}

HttpServer::~HttpServer() {
    ;
}

void HttpServer::handle_client(Socket::ptr client, void*) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        HttpRequest::ptr req = session->recv_request();
        if (!req) {
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "client close: " << *client;
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->get_version(), !is_keepalive_ || req->is_auto_close()));
        rsp->set_header("Server", get_name());
        // TODO: process request and generate response
        rsp->set_status(HttpStatus::OK);
        rsp->set_body("Hello from Sylar HttpServer, 114514\nWhat can I say?\nMamba out!");
        session->send_response(rsp);
    } while (is_keepalive_);
    session->close();
}



}
}