#include "http/http.h"
#include "log.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("http_test");

void test_request(){
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test_request";

    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->set_method(sylar::http::HttpMethod::GET);
    req->set_path("/");
    req->set_header("host", "www.baidu.com");
    req->set_auto_close(false);
    //req->set_body("hello world");

    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << '\n' << *req;
}

void test_response(){
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test_response";

    sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
    rsp->set_status(sylar::http::HttpStatus::NOT_FOUND);
    rsp->set_header("server", "sylar/1.0.0");
    rsp->set_body("<html><body><h1>404 Not Found</h1></body></html>");

    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << '\n' << *rsp;
}

int main(){
    test_request();
    test_response();
    return 0;
}