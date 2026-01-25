#include "http/http_session.h"
#include "http/http_server.h"
#include "tcp_server.h"
#include "iomanager.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

class EchoServer : public sylar::TcpServer{
public:
    void handle_client(sylar::Socket::ptr client, void*) override{
        sylar::http::HttpSession::ptr session(new sylar::http::HttpSession(client));
        while(true){
            sylar::http::HttpRequest::ptr req = session->recv_request();
            if(!req){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "client close: " << *client;
                break;
            }
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "request: \n" << *req;

            sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse(req->get_version(), req->is_auto_close()));
            rsp->set_status(sylar::http::HttpStatus::OK);
            rsp->set_body("hello sylar http server");
            rsp->set_header("Server", "sylar/0.1.0");
            session->send_response(rsp);
        }
    }
};

void test(void* arg = nullptr){
    EchoServer::ptr es(new EchoServer);
    es->set_name("sylar_http_session");
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8020");
    es->bind(addr);
    es->start();
}

void test_server(void* arg = nullptr){
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    server->set_name("sylar_http_server");
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8030");
    server->bind(addr);
    server->start();
}

int main(){
    sylar::IOManager iom(2);
    iom.add_fiber(test);
    iom.add_fiber(test_server);
    iom.start();
    return 0;
}