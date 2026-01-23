#ifndef __HTTP_HTTP_PARSER_H__
#define __HTTP_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar{
namespace http{

class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
public:
    HttpRequestParser();

    size_t execute(char* data, size_t len);
    size_t execute(std::string& data);
    int is_finished();
    int has_error();
    void set_errno(int v) { errno_ = v; }
    uint64_t get_content_length();
    HttpRequest::ptr get_request() const { request_->init_headers(); return request_; }
    const http_parser& get_parser() const { return parser_; }

    static uint64_t get_request_buffer_size();
    static uint64_t get_request_max_body_size();
private:
    http_parser parser_;
    HttpRequest::ptr request_;
    int errno_;
};

class HttpResponseParser{
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
public:
    HttpResponseParser();

    size_t execute(char* data, size_t len, bool chunked = false);
    size_t execute(std::string& data);
    int is_finished();
    int has_error();
    void set_errno(int v) { errno_ = v; }
    uint64_t get_content_length();
    HttpResponse::ptr get_response() const { return response_; }
    const httpclient_parser& get_parser() const { return parser_; }

    static uint64_t get_response_buffer_size();
    static uint64_t get_response_max_body_size();
private:
    httpclient_parser parser_;
    HttpResponse::ptr response_;
    int errno_;
};


}
}

#endif