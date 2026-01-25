#ifndef __SYLAR_HTTP_HTTP_SERVLET_H__
#define __SYLAR_HTTP_HTTP_SERVLET_H__

#include "mutex.h"
#include "http/http.h"
#include "http/http_session.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace sylar{
namespace http{

class HttpServlet{
public:
    typedef std::shared_ptr<HttpServlet> ptr;
public:
    HttpServlet(const std::string& name = "default http servlet");
    virtual ~HttpServlet() = default;

    virtual bool handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) = 0;

    const std::string& get_name() const { return name_; }
private:
    std::string name_;
};

class FunctionHttpServlet : public HttpServlet{
public:
    typedef std::shared_ptr<FunctionHttpServlet> ptr;
    typedef std::function<bool(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)> CallbackType;
public:
    FunctionHttpServlet(CallbackType cb, const std::string& name = "function http servlet");
    virtual ~FunctionHttpServlet() = default;

    bool handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;
private:
    CallbackType cb_;
};

class ServletDispatch : public HttpServlet{
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
public:
    ServletDispatch(const std::string& name = "servlet dispatch");
    virtual ~ServletDispatch() = default;

    virtual bool handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

    HttpServlet::ptr get_default_servlet() const;
    void set_default_servlet(HttpServlet::ptr v);

    HttpServlet::ptr get_servlet(const std::string& url);
    HttpServlet::ptr get_exact_servlet(const std::string& url);
    HttpServlet::ptr get_fuzzy_servlet(const std::string& url);

    void add_exact_servlet(const std::string& url, HttpServlet::ptr servlet);
    void add_exact_servlet(const std::string& url, FunctionHttpServlet::CallbackType cb);
    void add_fuzzy_servlet(const std::string& url, HttpServlet::ptr servlet);
    void add_fuzzy_servlet(const std::string& url, FunctionHttpServlet::CallbackType cb);

    bool del_exact_servlet(const std::string& url);
    bool del_fuzzy_servlet(const std::string& url);
private:
    mutable RWMutex rw_mutex_;
    HttpServlet::ptr default_servlet_;
    std::unordered_map<std::string, HttpServlet::ptr> exact_matchs_;
    std::vector<std::pair<std::string, HttpServlet::ptr>> fuzzy_matchs_;
};

class NotFoundServlet : public HttpServlet{
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
public:
    NotFoundServlet(const std::string& name = "not found servlet");
    virtual ~NotFoundServlet() = default;
    virtual bool handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;
};



}
}

#endif