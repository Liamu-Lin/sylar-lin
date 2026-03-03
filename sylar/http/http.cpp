#include "http/http.h"
#include <string.h>

namespace sylar{
namespace http{

HttpMethod string_to_http_method(const std::string& m) {
#define XX(code, name, string)  \
    if(#string == m)            \
        return HttpMethod::name;
    HTTP_METHOD_MAP(XX)
#undef XX
    return HttpMethod::HTTP_INVALID_METHOD;
}
HttpMethod string_to_http_method(const char* m) {
#define XX(code, name, string)      \
    if(strcmp(m, #string) == 0)     \
        return HttpMethod::name;
    HTTP_METHOD_MAP(XX)
#undef XX
    return HttpMethod::HTTP_INVALID_METHOD;
}

const char* http_method_to_string(const HttpMethod& m) {
#define XX(code, name, string)  \
    case HttpMethod::name:    \
        return #string;
    switch(m){
        HTTP_METHOD_MAP(XX)
        default:
            return "<unknown>";
    }
#undef XX
}
const char* http_status_to_string(const HttpStatus& s) {
#define XX(code, name, string)  \
    case HttpStatus::name:      \
        return #string;
    switch(s){
        HTTP_STATUS_MAP(XX)
        default:
            return "<unknown>";
    }
#undef XX
}


HttpRequest::HttpRequest(uint8_t version, bool auto_close):
    auto_close_(auto_close),
    is_websocket_(false),
    parse_flags_(0),
    method_(HttpMethod::GET),
    version_(version),
    path_("/"){
    init_headers();
}

void HttpRequest::init_headers(){
    if(!is_websocket())
        set_header("connection", is_auto_close() ? "close" : "keep-alive");
}

void HttpRequest::set_auto_close(bool v) { 
    if(auto_close_ == v)
        return;
    auto_close_ = v;
    if(!is_websocket_)
        set_header("connection", is_auto_close() ? "close" : "keep-alive");
}

void HttpRequest::set_websocket(bool v) {
    if(is_websocket_ == v)
        return;
    is_websocket_ = v;
    if(is_websocket_)
        del_header("connection");
    else
        set_header("connection", is_auto_close() ? "close" : "keep-alive");
}

void HttpRequest::set_body(const std::string& v){ 
    set_header("content-length", std::to_string(v.size()));
    body_ = v;
}

std::string HttpRequest::get_header(const std::string& name) const{
    auto it = headers_.find(name);
    if(it != headers_.end())
        return it->second;
    return "";
}
void HttpRequest::del_header(const std::string& name){
    headers_.erase(name);
}
void HttpRequest::set_header(const std::string& name, const std::string& value){
    if(strcasecmp(name.c_str(), "connection") == 0){
        if(strcasecmp(value.c_str(), "close") == 0)
            set_auto_close(true);
        else
            set_auto_close(false);
    }
    headers_[name] = value;
}

std::string HttpRequest::get_param(const std::string& name) const{
    auto it = params_.find(name);
    if(it != params_.end())
        return it->second;
    return "";
}
void HttpRequest::del_param(const std::string& name){
    params_.erase(name);
}
void HttpRequest::set_param(const std::string& name, const std::string& value){
    params_[name] = value;
}

std::string HttpRequest::get_cookie(const std::string& name) const{
    auto it = cookies_.find(name);
    if(it != cookies_.end())
        return it->second;
    return "";
}
void HttpRequest::del_cookie(const std::string& name){
    cookies_.erase(name);
}
void HttpRequest::set_cookie(const std::string& name, const std::string& value){
    cookies_[name] = value;
}

std::string HttpRequest::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}
std::ostream& operator<<(std::ostream& os, const HttpRequest& request){
    os << http_method_to_string(request.method_) << ' '
       << request.path_
       << (request.query_.empty() ? "" : "?") << request.query_
       << (request.fragment_.empty() ? "" : "#") << request.fragment_
       << " HTTP/" << (uint32_t(request.version_) >> 4) << '.' << (uint32_t(request.version_) & 0x0F)
       << "\r\n";
    for(auto& i : request.headers_)
        os << i.first << ": " << i.second << "\r\n";
    if(!request.body_.empty())
        os << "\r\n" << request.body_;
    else
        os << "\r\n";
    return os;
}


HttpResponse::HttpResponse(uint8_t version, bool close):
    close_(close),
    is_websocket_(false),
    version_(version),
    status_(HttpStatus::OK){
    init_headers();
}

void HttpResponse::init_headers(){
    if(!is_websocket())
        set_header("connection", is_close() ? "close" : "keep-alive");
}

void HttpResponse::set_close(bool v){
    if(close_ == v)
        return;
    close_ = v;
    if(!is_websocket_)
        set_header("connection", is_close() ? "close" : "keep-alive");
}

void HttpResponse::set_websocket(bool v){
    if(is_websocket_ == v)
        return;
    is_websocket_ = v;
    if(is_websocket_)
        del_header("connection");
    else
        set_header("connection", is_close() ? "close" : "keep-alive");
}

void HttpResponse::set_body(const std::string& v){
    set_header("content-length", std::to_string(v.size()));
    body_ = v;
}

std::string HttpResponse::get_header(const std::string& name) const{
    auto it = headers_.find(name);
    if(it != headers_.end())
        return it->second;
    return "";
}
void HttpResponse::del_header(const std::string& name){
    headers_.erase(name);
}
void HttpResponse::set_header(const std::string& name, const std::string& value){
    headers_[name] = value;
}

std::string HttpResponse::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}
std::ostream& operator<<(std::ostream& os, const HttpResponse& response){
    os << "HTTP/" << (uint32_t(response.version_) >> 4) << '.' << (uint32_t(response.version_) & 0x0F)
       << ' ' << static_cast<uint32_t>(response.status_) << ' ' << http_status_to_string(response.status_) << "\r\n";
    for(auto& i : response.headers_)
        os << i.first << ": " << i.second << "\r\n";
    for(auto& i : response.cookies_)
        os << "Set-Cookie: " << i << "\r\n";
    if(!response.body_.empty())
        os << "\r\n" << response.body_;
    else
        os << "\r\n";
    return os;
}

}
}