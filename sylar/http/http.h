#ifndef __HTTP_HTTP_H__
#define __HTTP_HTTP_H__

#include "type.h"

#include <boost/lexical_cast.hpp>
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace sylar {
namespace http{

// Request Methods
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)

// Status Codes
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)


enum class HttpMethod{
#define XX(code, name, string) name = code,
    HTTP_METHOD_MAP(XX)
#undef XX
    HTTP_INVALID_METHOD
};

enum class HttpStatus{
#define XX(code, name, string) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
    HTTP_INVALID_STATUS
};

typedef std::map<std::string, std::string, sylar::CaseInsensitiveLess> CaseInsensitiveStr2StrMap;
typedef CaseInsensitiveStr2StrMap HttpHeaders;
typedef CaseInsensitiveStr2StrMap HttpParams;
typedef CaseInsensitiveStr2StrMap HttpCookies;
template<typename T>
static T get_value_as(const CaseInsensitiveStr2StrMap& map, const std::string& key, const T& default_value = T()){
    auto it = map.find(key);
    if(it == map.end())
        return default_value;
    try{
        return boost::lexical_cast<T>(it->second);
    }catch(...){
        return default_value;
    }
}

class HttpRequest{
public:
    typedef std::shared_ptr<HttpRequest> ptr;
public:
    HttpRequest(uint8_t version = 0x11, bool auto_close = true);

    bool is_auto_close() const { return auto_close_; }
    void set_auto_close(bool v);

    bool is_websocket() const { return is_websocket_; }
    void set_websocket(bool v);

    HttpMethod get_method() const { return method_; }
    void set_method(HttpMethod v) { method_ = v; }

    uint8_t get_version() const { return version_; }
    void set_version(uint8_t v) { version_ = v; }


    const std::string& get_path() const { return path_; }
    void set_path(const std::string& v) { path_ = v; }

    const std::string& get_query() const { return query_; }
    void set_query(const std::string& v) { query_ = v; }

    const std::string& get_fragment() const { return fragment_; }
    void set_fragment(const std::string& v) { fragment_ = v; }

    const std::string& get_body() const { return body_; }
    void set_body(const std::string& v);

    const HttpHeaders& get_headers() const { return headers_; }
    void set_headers(const HttpHeaders& v) { headers_ = v; }

    const HttpParams& get_params() const { return params_; }
    void set_params(const HttpParams& v) { params_ = v; }

    const HttpCookies& get_cookies() const { return cookies_; }
    void set_cookies(const HttpCookies& v) { cookies_ = v; }

    std::string get_header(const std::string& name) const;
    void del_header(const std::string& name);
    void set_header(const std::string& name, const std::string& value);

    std::string get_param(const std::string& name) const;
    void del_param(const std::string& name);
    void set_param(const std::string& name, const std::string& value);

    std::string get_cookie(const std::string& name) const;
    void del_cookie(const std::string& name);
    void set_cookie(const std::string& name, const std::string& value);

    template<typename T>
    T get_header_as(const std::string& name, const T& default_value = T()) const {
        return sylar::http::get_value_as<T>(headers_, name, default_value);
    }
    template<typename T>
    T get_param_as(const std::string& name, const T& default_value = T()) const {
        return sylar::http::get_value_as<T>(params_, name, default_value);
    }
    template<typename T>
    T get_cookie_as(const std::string& name, const T& default_value = T()) const {
        return sylar::http::get_value_as<T>(cookies_, name, default_value);
    }

    friend std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

    void init_headers();
private:
    // TODO
    //void init_params();
    //void init_cookies();
private:
    bool auto_close_;
    bool is_websocket_;
    uint8_t parse_flags_;

    HttpMethod method_;
    uint8_t version_;

    std::string path_;
    std::string query_;
    std::string fragment_;
    std::string body_;

    HttpHeaders headers_;
    HttpParams params_;
    HttpCookies cookies_;
};

class HttpResponse{
public:
    typedef std::shared_ptr<HttpResponse> ptr;
public:
    HttpResponse(uint8_t version = 0x11, bool close = false);

    bool is_close() const { return close_; }
    void set_close(bool v);

    bool is_websocket() const { return is_websocket_; }
    void set_websocket(bool v);

    HttpStatus get_status() const { return status_; }
    void set_status(HttpStatus v) { status_ = v; }

    const std::string& get_reason() const { return reason_; }
    void set_reason(const std::string& v) { reason_ = v; }

    uint8_t get_version() const { return version_; }
    void set_version(uint8_t v) { version_ = v; }

    const std::string& get_body() const { return body_; }
    void set_body(const std::string& v);

    const HttpHeaders& get_headers() const { return headers_; }
    void set_headers(const HttpHeaders& v) { headers_ = v; }

    std::string get_header(const std::string& name) const;
    void del_header(const std::string& name);
    void set_header(const std::string& name, const std::string& value);

    template<typename T>
    T get_header_as(const std::string& name, const T& default_value = T()) const {
        return sylar::http::get_value_as<T>(headers_, name, default_value);
    }

    friend std::ostream& operator<<(std::ostream& os, const HttpResponse& response);

    void init_headers();
private:
    bool close_;
    bool is_websocket_;    

    uint8_t version_;
    HttpStatus status_;
    std::string reason_;
    std::string body_;
    HttpHeaders headers_;
    std::vector<std::string> cookies_;
};



HttpMethod string_to_http_method(const std::string& m);
HttpMethod string_to_http_method(const char* m);
const char* http_method_to_string(const HttpMethod& m);
const char* http_status_to_string(const HttpStatus& s); 

}
}


#endif