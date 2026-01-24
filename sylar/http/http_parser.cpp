#include "http_parser.h"
#include "config.h"


namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

static sylar::ConfigVar<uint64_t>::ptr g_http_request_buffer_size = 
            sylar::ConfigMgr.look_up<uint64_t>("http.request.buffer_size", 4 * 1024, "http request buffer size");
static sylar::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
            sylar::ConfigMgr.look_up<uint64_t>("http.request.max_body_size", 64 * 1024 * 1024, "http request max body size");
static sylar::ConfigVar<uint64_t>::ptr g_http_response_buffer_size = 
            sylar::ConfigMgr.look_up<uint64_t>("http.response.buffer_size", 4 * 1024, "http response buffer size");
static sylar::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
            sylar::ConfigMgr.look_up<uint64_t>("http.response.max_body_size", 64 * 1024 * 1024, "http response max body size");
uint64_t t_http_request_buffer_size = 4 * 1024;
uint64_t t_http_request_max_body_size = 64 * 1024 * 1024;
uint64_t t_http_response_buffer_size = 4 * 1024;
uint64_t t_http_response_max_body_size = 64 * 1024 * 1024;

namespace{
struct _HttpRequestSizeIniter{
    _HttpRequestSizeIniter(){
        t_http_request_buffer_size = g_http_request_buffer_size->get_value();
        t_http_request_max_body_size = g_http_request_max_body_size->get_value();
        t_http_response_buffer_size = g_http_response_buffer_size->get_value();
        t_http_response_max_body_size = g_http_response_max_body_size->get_value();

        g_http_request_buffer_size->add_listener([](const uint64_t& old_value, const uint64_t& new_value){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "http.request.buffer_size changed from "
                << old_value << " to " << new_value;
            t_http_request_buffer_size = new_value;
        });
        g_http_request_max_body_size->add_listener([](const uint64_t& old_value, const uint64_t& new_value){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "http.request.max_body_size changed from "
                << old_value << " to " << new_value;
            t_http_request_max_body_size = new_value;
        });
        g_http_response_buffer_size->add_listener([](const uint64_t& old_value, const uint64_t& new_value){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "http.response.buffer_size changed from "
                << old_value << " to " << new_value;
            t_http_response_buffer_size = new_value;
        });
        g_http_response_max_body_size->add_listener([](const uint64_t& old_value, const uint64_t& new_value){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "http.response.max_body_size changed from "
                << old_value << " to " << new_value;
            t_http_response_max_body_size = new_value;
        });
    }
};
static _HttpRequestSizeIniter _init;
}

uint64_t HttpRequestParser::get_request_buffer_size(){
    return t_http_request_buffer_size;
}
uint64_t HttpRequestParser::get_request_max_body_size(){
    return t_http_request_max_body_size;
}
uint64_t HttpResponseParser::get_response_buffer_size(){
    return t_http_response_buffer_size;
}
uint64_t HttpResponseParser::get_response_max_body_size(){
    return t_http_response_max_body_size;
}

static void nop_callback(void* _, const char* __, size_t ___){
}
void on_request_method(void* data, const char* at, size_t length){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    HttpMethod m = string_to_http_method(std::string(at, length));
    parser->get_request()->set_method(m);
    if(m == HttpMethod::HTTP_INVALID_METHOD)
        parser->set_errno(0x1002); // invalid http method
}
void on_request_fragment(void* data, const char* at, size_t length){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    parser->get_request()->set_fragment(std::string(at, length));
}
void on_request_path(void* data, const char* at, size_t length){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    parser->get_request()->set_path(std::string(at, length));
}
void on_request_query(void* data, const char* at, size_t length){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    parser->get_request()->set_query(std::string(at, length));
}
void on_request_version(void* data, const char* at, size_t length){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    if(strncmp(at, "HTTP/1.1", length) == 0)
        parser->get_request()->set_version(0x11);
    else if(strncmp(at, "HTTP/1.0", length) == 0)
        parser->get_request()->set_version(0x10);
    else
        parser->set_errno(0x1001); // invalid http version
}
static void on_request_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen){
    HttpRequestParser* parser = (HttpRequestParser*)data;
    std::string f(field, flen);
    std::string v(value, vlen);
    parser->get_request()->set_header(f, v);
}

HttpRequestParser::HttpRequestParser():
    request_(std::make_shared<HttpRequest>()),
    errno_(0){
    http_parser_init(&parser_);
    parser_.data = this;
    parser_.request_method = on_request_method;
    parser_.request_uri = nop_callback;
    parser_.fragment = on_request_fragment;
    parser_.request_path = on_request_path;
    parser_.query_string = on_request_query;
    parser_.http_version = on_request_version;
    parser_.header_done = nop_callback;
    parser_.http_field = on_request_http_field;
}

bool HttpRequestParser::execute(const char* data, size_t len, size_t& offset){
    offset = http_parser_execute(&parser_, data, len, offset);
    bool finished = http_parser_is_finished(&parser_);
    return finished;
}
bool HttpRequestParser::execute(const std::string& data, size_t& offset){
    offset = http_parser_execute(&parser_, data.c_str(), data.size(), offset);
    bool finished = http_parser_is_finished(&parser_);
    return finished;
}

int HttpRequestParser::is_finished(){
    return http_parser_is_finished(&parser_);
}

int HttpRequestParser::has_error(){
    return errno_ || http_parser_has_error(&parser_);
}

uint64_t HttpRequestParser::get_content_length(){
    if(parser_.content_len > 0)
        return parser_.content_len;
    else
        return request_->get_header_as<uint64_t>("content-length", 0);
}



void on_response_reason(void* data, const char* at, size_t length){
    HttpResponseParser* parser = (HttpResponseParser*)data;
    parser->get_response()->set_reason(std::string(at, length));
}
void on_response_status(void* data, const char* at, size_t length){
    HttpResponseParser* parser = (HttpResponseParser*)data;
    HttpStatus s = (HttpStatus)atoi(std::string(at, length).c_str());
    parser->get_response()->set_status(s);
}
void on_response_version(void* data, const char* at, size_t length){
    HttpResponseParser* parser = (HttpResponseParser*)data;
    if(strncmp(at, "HTTP/1.1", length) == 0)
        parser->get_response()->set_version(0x11);
    else if(strncmp(at, "HTTP/1.0", length) == 0)
        parser->get_response()->set_version(0x10);
    else
        parser->set_errno(0x1001); // invalid http version
}
void on_response_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen){
    HttpResponseParser* parser = (HttpResponseParser*)data;
    std::string f(field, flen);
    std::string v(value, vlen);
    parser->get_response()->set_header(f, v);
    if(f == "transfer-encoding" && v == "chunked"){
        parser->is_chunked_ = true;
    }
}


HttpResponseParser::HttpResponseParser():
    response_(std::make_shared<HttpResponse>()),
    errno_(0),
    is_chunked_(false){
    httpclient_parser_init(&parser_);
    parser_.data = this;
    parser_.reason_phrase = on_response_reason;
    parser_.status_code = on_response_status;
    parser_.chunk_size = nop_callback;
    parser_.http_version = on_response_version;
    parser_.header_done = nop_callback;
    parser_.last_chunk = nop_callback;
    parser_.http_field = on_response_http_field;
}

bool HttpResponseParser::execute(const char* data, size_t len, size_t& offset){
    offset = httpclient_parser_execute(&parser_, data, len, offset);
    bool finished = httpclient_parser_is_finished(&parser_);
    return finished;
}
bool HttpResponseParser::execute(const std::string& data, size_t& offset){
    offset = httpclient_parser_execute(&parser_, data.c_str(), data.size(), offset);
    bool finished = httpclient_parser_is_finished(&parser_);
    return finished;
}   

int HttpResponseParser::is_finished(){
    return httpclient_parser_is_finished(&parser_);
}
bool HttpResponseParser::is_chunked(){
    return is_chunked_ || parser_.chunked;
}
int HttpResponseParser::has_error(){
    return errno_ || httpclient_parser_has_error(&parser_);
}

uint64_t HttpResponseParser::get_content_length(){
    if(parser_.content_len > 0)
        return parser_.content_len;
    else
        return response_->get_header_as<uint64_t>("content-length", 0);
}


}
}