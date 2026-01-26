#include "log.h"
#include "iomanager.h"
#include "http/http_connection.h"

void test_connection(void* arg = nullptr){
    //sylar::Address::ptr addr = sylar::Address::lookup_address("www.baidu.com:80");
    sylar::Address::ptr addr = sylar::Address::lookup_address("www.httpbin.org:80", AF_INET);
    if(!addr){
        SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::ERROR)
            << "lookup httpbin.org:80 failed";
        return;
    }
    sylar::Socket::ptr sock(new sylar::Socket(addr->get_family(), SOCK_STREAM, 0));
    bool rt = sock->connect(addr);
    if(!rt){
        SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::ERROR)
            << "connect " << addr->to_string() << " failed";
        return;
    }

    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->set_version(0x11);
    req->set_path("/stream/20");
    req->set_header("Host", "httpbin.org");
    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "request:\n" << *req;
    
    conn->send_request(req);
    sylar::http::HttpResponse::ptr rsp = conn->recv_response();
    if(!rsp){
        SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::ERROR)
            << "recv response failed";
        return;
    }

    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "response:\n" << *rsp;
}

void nop(void* arg){
}

int main(){
    sylar::IOManager iom(2);

    //iom.add_fiber(nop);
    iom.add_fiber(test_connection);
    iom.start();

    return 0;
}