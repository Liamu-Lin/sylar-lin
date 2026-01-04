#include "address.h"

namespace sylar{

std::shared_ptr<Logger> g_logger = LoggerMgr.get_logger("system");
/*
std::shared_ptr<Address> Address::create(const sockaddr* addr, socklen_t addr_len){
    if(!addr)
        return nullptr;
    switch(addr->sa_family){
        case AF_INET:
            return std::make_shared<IPv4Address>(*(const sockaddr_in*)addr);
        case AF_INET6:
            return std::make_shared<IPv6Address>(*(const sockaddr_in6*)addr);
        default:
            return std::make_shared<UnknownAddress>(*addr);
    }
    return nullptr;
}

bool Address::get_host_addresses(std::vector<std::shared_ptr<Address>>& result, const std::string& host, int family, int socket_type, int protocol){
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socket_type;
    hints.ai_protocol = protocol;

    std::string node, service;
    if(!host.empty()){
        // ipv6 -- [host]:port/service
        if(host[0] == '['){
            size_t node_end = host.find(']');
            if(node_end != std::string::npos){
                node = host.substr(1, node_end - 1);
                if(host.length() > node_end + 2)
                    service = host.substr(node_end + 2);
            }
        }
        // ipv4 -- host:port/service
        else{
            size_t node_end = host.find(':');
            if(node_end != std::string::npos){
                node = host.substr(0, node_end);
                if(host.length() > node_end + 1)
                    service = host.substr(node_end + 1);
            }
        }
    }
    if(node.empty())
        node = host;

    addrinfo* results;
    int error = getaddrinfo(node.c_str(), service.c_str(), &hints, &results);
    if(error){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Address::get_host_addresses getaddrinfo(" << host << ", " << family
            << ", " << socket_type << ", " << protocol << ") error=" << error
            << " errstr=" << gai_strerror(error);
        return false;
    }

    for(addrinfo* cur = results; cur; cur = cur->ai_next){
        std::shared_ptr<Address> addr = Address::create(cur->ai_addr, (socklen_t)cur->ai_addrlen);
        if(addr)
            result.push_back(addr);
    }

    freeaddrinfo(results);
    return !result.empty();
}
*/



}