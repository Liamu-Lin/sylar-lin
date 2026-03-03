#include "http/http_server.h"
#include "log.h"
#include "iomanager.h"

#define XX(...) #__VA_ARGS__

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

void test_echo(void* arg = nullptr){
    EchoServer::ptr es(new EchoServer);
    es->set_name("sylar_http_session");
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8020");
    es->bind(addr);
    es->start();
}

void test_server(void* arg = nullptr){
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    server->set_name("sylar_http_server");
    server->set_keepalive(true);
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8030");
    server->bind(addr);

    auto sd = server->get_servlet_dispatch();
    sd->add_exact_servlet("/sylar/xx", [](sylar::http::HttpRequest::ptr req,
        sylar::http::HttpResponse::ptr rsp,
        sylar::http::HttpSession::ptr session) {
            rsp->set_body(req->to_string());
            return true;
    });

    sd->add_fuzzy_servlet("/sylar/*", [](sylar::http::HttpRequest::ptr req,
        sylar::http::HttpResponse::ptr rsp,
        sylar::http::HttpSession::ptr session) {
            rsp->set_body("Fuzzy:\r\n" + req->to_string());
            return true;
    });

    sd->add_fuzzy_servlet("/sylarx/*", [](sylar::http::HttpRequest::ptr req,
        sylar::http::HttpResponse::ptr rsp,
        sylar::http::HttpSession::ptr session) {
            rsp->set_body(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return true;
    });


    server->start();
}

int main(){
    sylar::IOManager iom(2);
    
    //iom.add_fiber(test_echo);
    g_logger->set_level(sylar::LogLevel::Level::WARN);
    //g_logger->set_level(sylar::LogLevel::Level::INFO);
    iom.add_fiber(test_server);
    
    iom.start();

    return 0;
}