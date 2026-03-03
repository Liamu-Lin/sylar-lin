#include "http/http_server.h"
#include "log.h"

namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

HttpServer::HttpServer(IOManager* worker, IOManager* accept_worker)
    :TcpServer(worker, accept_worker),
    is_keepalive_(false),
    servlet_dispatch_(new ServletDispatch()){
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
        // generate response
        HttpResponse::ptr rsp(new HttpResponse(req->get_version(), !is_keepalive_ || req->is_auto_close()));
        rsp->set_header("Server", get_name());
        if (!servlet_dispatch_->handle_request(req, rsp, session)) {
            SYLAR_LOG(g_logger, LogLevel::Level::WARN) << "request handle failed: " << *req;
            rsp->set_status(HttpStatus::INTERNAL_SERVER_ERROR);
            rsp->set_body("500 Internal Server Error");
        }
        // send response
        session->send_response(rsp);
        if(req->is_auto_close()){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "auto close: " << *client;
        }
        if(rsp->is_close()) break;
    } while (true);
    session->close();
}



}
}