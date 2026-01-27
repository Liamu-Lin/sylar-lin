#ifndef __SYLAR_URI_H__
#define __SYLAR_URI_H__

#include "address.h"

#include <stdint.h>
#include <memory>
#include <string>

namespace sylar{

class URI{
public:
    typedef std::shared_ptr<URI> ptr;
    static URI::ptr create(const std::string& uri_str);
public:
    URI();
    URI(const std::string& uri_str);
    ~URI() = default;

    const std::string& get_scheme() const { return scheme_; }
    const std::string& get_user() const { return user_; }
    const std::string& get_host() const { return host_; }
    uint16_t get_port() const;
    const std::string& get_path() const;
    const std::string& get_query() const { return query_; }
    const std::string& get_fragment() const { return fragment_; }

    Address::ptr get_address() const;

    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const URI& uri);
private:
    void set_scheme(const std::string& v) { scheme_ = v; }
    void set_user(const std::string& v) { user_ = v; }
    void set_host(const std::string& v) { host_ = v; }
    void set_port(uint16_t v) { port_ = v; }
    void set_path(const std::string& v) { path_ = v; }
    void set_query(const std::string& v) { query_ = v; }
    void set_fragment(const std::string& v) { fragment_ = v; }

    bool is_default_port() const;
private:
    uint16_t port_;
    std::string scheme_;
    std::string user_;
    std::string host_;
    std::string path_;
    std::string query_;
    std::string fragment_;
};

}

#endif