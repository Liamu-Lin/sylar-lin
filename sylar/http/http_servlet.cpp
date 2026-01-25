#include "http/http_servlet.h"
#include <fnmatch.h>

namespace sylar{
namespace http{

HttpServlet::HttpServlet(const std::string& name)
    :name_(name){
    ;
}



FunctionHttpServlet::FunctionHttpServlet(CallbackType cb, const std::string& name):
    HttpServlet(name),
    cb_(cb){
    if(!cb_){
        throw std::invalid_argument("cb is null");
    }
}

bool FunctionHttpServlet::handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session){
    return cb_(request, response, session);
}



ServletDispatch::ServletDispatch(const std::string& name):
    HttpServlet(name),
    default_servlet_(std::make_shared<NotFoundServlet>()){
    ;
}

bool ServletDispatch::handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session){
    HttpServlet::ptr matched_servlet = get_servlet(request->get_path());
    if(!matched_servlet)
        matched_servlet = get_default_servlet();
    if(matched_servlet)
        return matched_servlet->handle_request(request, response, session);
    return false;
}

HttpServlet::ptr ServletDispatch::get_default_servlet() const {
    RWMutex::RLock lock(rw_mutex_);
    return default_servlet_;
}
void ServletDispatch::set_default_servlet(HttpServlet::ptr v){
    RWMutex::WLock lock(rw_mutex_);
    default_servlet_ = v;
}

HttpServlet::ptr ServletDispatch::get_servlet(const std::string& url){
    HttpServlet::ptr servlet = get_exact_servlet(url);
    if(servlet)
        return servlet;
    return get_fuzzy_servlet(url);
}
HttpServlet::ptr ServletDispatch::get_exact_servlet(const std::string& url){
    RWMutex::RLock lock(rw_mutex_);
    auto it = exact_matchs_.find(url);
    return it == exact_matchs_.end() ? nullptr : it->second;
}
HttpServlet::ptr ServletDispatch::get_fuzzy_servlet(const std::string& url){
    RWMutex::RLock lock(rw_mutex_);
    for(auto& v : fuzzy_matchs_){
        if(fnmatch(v.first.c_str(), url.c_str(), 0) == 0)
            return v.second;
    }
    return nullptr;
}

void ServletDispatch::add_exact_servlet(const std::string& url, HttpServlet::ptr servlet){
    RWMutex::WLock lock(rw_mutex_);
    exact_matchs_[url] = servlet;
}
void ServletDispatch::add_exact_servlet(const std::string& url, FunctionHttpServlet::CallbackType cb){
    add_exact_servlet(url, std::make_shared<FunctionHttpServlet>(cb));
}
void ServletDispatch::add_fuzzy_servlet(const std::string& url, HttpServlet::ptr servlet){
    RWMutex::WLock lock(rw_mutex_);
    for(auto it = fuzzy_matchs_.begin();
        it != fuzzy_matchs_.end(); ++it){
        if(it->first == url){
            it->second = servlet;
            return;
        }
    }
    fuzzy_matchs_.emplace_back(url, servlet);
}
void ServletDispatch::add_fuzzy_servlet(const std::string& url, FunctionHttpServlet::CallbackType cb){
    add_fuzzy_servlet(url, std::make_shared<FunctionHttpServlet>(cb));
}

bool ServletDispatch::del_exact_servlet(const std::string& url){
    RWMutex::WLock lock(rw_mutex_);
    if(exact_matchs_.find(url) == exact_matchs_.end())
        return false;
    exact_matchs_.erase(url);
    return true;
}
bool ServletDispatch::del_fuzzy_servlet(const std::string& url){
    RWMutex::WLock lock(rw_mutex_);
    for(auto it = fuzzy_matchs_.begin(); it != fuzzy_matchs_.end(); ++it){
        if(it->first == url){
            fuzzy_matchs_.erase(it);
            return true;
        }
    }
    return false;
}



NotFoundServlet::NotFoundServlet(const std::string& name):
    HttpServlet(name){
    ;
}

bool NotFoundServlet::handle_request(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session){
    response->set_status(HttpStatus::NOT_FOUND);
    response->set_header("Server", "sylar/1.0.0");
    response->set_header("Content-Type", "text/html");
    response->set_body("<html><head><title>404 Not Found</title></head>"
                      "<body><center><h1>404 Not Found</h1></center>"
                      "<hr><center>sylar http server</center></body></html>");
    return true;
}


}
}