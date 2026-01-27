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

    bool execute(const char* data, size_t len, size_t& offset);
    bool execute(const std::string& data, size_t& offset);

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
private:
    friend void on_response_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen);
public:
    HttpResponseParser();

    bool execute(const char* data, size_t len, size_t& offset);
    bool execute(const std::string& data, size_t& offset);

    int is_finished();
    bool is_chunked() const { return is_chunked_ || parser_.chunked; }
    bool is_chunk_done() const { return parser_.chunks_done; }

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
    bool is_chunked_;
};


}
}

#endif