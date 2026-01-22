#include "http/http_parser.h"
#include "log.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                "Host: www.sylar.top\r\n"
                                "Connection: keep-alive\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";
void test_request_parser(){
    sylar::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t offset = parser.execute(tmp);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "\nexecute offset=" << offset
        << "\nis_finished=" << parser.is_finished()
        << "\nhas_error=" << parser.has_error()
        << "\ncontent_length=" << parser.get_content_length()
        << "\nrequest=\n" << *(parser.get_request())
        << "\n" << tmp;//.substr(offset);
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                  "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
                                  "Server: Apache/2.2.14 (Win32)\r\n"
                                  "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Content-Length: 81\r\n"
                                  "Cache-Control: max-age=86400\r\n"
                                  "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
                                  "Connection: Close\r\n"
                                  "Content-Type: text/html\r\n\r\n"
                                  "<html>\r\n"
                                  "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                  "</html>\r\n";
void test_response_parser(){
    sylar::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t offset = parser.execute(tmp);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "\nexecute offset=" << offset
        << "\nis_finished=" << parser.is_finished()
        << "\nhas_error=" << parser.has_error()
        << "\ncontent_length=" << parser.get_content_length()
        << "\nresponse=\n" << *(parser.get_response())
        << "\n" << tmp;//.substr(offset);
}

int main(){

    test_request_parser();

    test_response_parser();

    return 0;
}