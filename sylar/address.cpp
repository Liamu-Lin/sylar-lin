#include "address.h"

namespace sylar{

static std::shared_ptr<Logger> g_logger = LoggerMgr.get_logger("system");

Address::ptr Address::create(int family){
    switch(family){
        case AF_INET:
            return std::make_shared<IPv4Address>();
        case AF_INET6:
            return std::make_shared<IPv6Address>();
        case AF_UNIX:
            return std::make_shared<UnixAddress>();
        default:
            return std::make_shared<UnknownAddress>(family);
    }
    return nullptr;
}
Address::ptr Address::create(const sockaddr* addr){
    if(!addr)
        return nullptr;
    switch(addr->sa_family){
        case AF_INET:
            return std::make_shared<IPv4Address>(*(const sockaddr_in*)addr);
        case AF_INET6:
            return std::make_shared<IPv6Address>(*(const sockaddr_in6*)addr);
        default:
            return std::make_shared<UnknownAddress>(*addr, sizeof(*addr));
    }
    return nullptr;
}

bool Address::lookup_address(std::vector<Address::ptr>& result, const std::string& host, int family, int socket_type, int protocol){
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
        std::shared_ptr<Address> addr = Address::create(cur->ai_addr);
        if(addr)
            result.push_back(addr);
    }

    freeaddrinfo(results);
    return !result.empty();
}
Address::ptr Address::lookup_address(const std::string& host, int family, int socket_type, int protocol){
    std::vector<Address::ptr> results;
    bool v = lookup_address(results, host, family, socket_type, protocol);
    if(v)
        return results[0];
    return nullptr;
}

bool IPAddress::lookup_ip_address(std::vector<IPAddress::ptr>& result, const std::string& host, int socket_type, int protocol){
    std::vector<Address::ptr> adds;
    bool v = Address::lookup_address(adds, host, AF_UNSPEC, socket_type, protocol);
    if(!v){
        return false;
    }
    for(auto& i : adds){
        IPAddress::ptr ip = std::dynamic_pointer_cast<IPAddress>(i);
        if(ip){
            result.push_back(ip);
        }
    }
    return !result.empty();
}
IPAddress::ptr IPAddress::lookup_ip_address(const std::string& host, int socket_type, int protocol){
    std::vector<IPAddress::ptr> results;
    bool v = lookup_ip_address(results, host, socket_type, protocol);
    if(v)
        return results[0];
    return nullptr;
}

bool Address::get_interface_address(std::unordered_map<std::string, std::vector<std::pair<Address::ptr, uint32_t>>>& result, int family){
    ifaddrs* ifap;
    if(getifaddrs(&ifap) != 0){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Address::get_interface_address getifaddrs fail"
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    for(ifaddrs* cur = ifap; cur; cur = cur->ifa_next){
        if(family != cur->ifa_addr->sa_family && family != AF_UNSPEC)
            continue;
        uint32_t prefix_len = 0;
        switch(cur->ifa_addr->sa_family){
            case AF_INET: {
                sockaddr_in* netmask = (sockaddr_in*)cur->ifa_netmask;
                uint32_t mask = ntohl(netmask->sin_addr.s_addr);
                while(mask){
                    mask &= (mask - 1);
                    ++prefix_len;
                }
                break;
            }
            case AF_INET6: {
                sockaddr_in6* netmask = (sockaddr_in6*)cur->ifa_netmask;
                for(int i = 0; i < 16; ++i){
                    uint8_t mask = netmask->sin6_addr.s6_addr[i];
                    while(mask){
                        mask &= (mask - 1);
                        ++prefix_len;
                    }
                }
                break;
            }
            default: break;
        }
        Address::ptr addr = Address::create(cur->ifa_addr);
        if(!addr || prefix_len == 0)
            continue;
        result[cur->ifa_name].push_back(std::make_pair(addr, prefix_len));
    }
    freeifaddrs(ifap);
    return !result.empty();
}

bool Address::get_interface_address(std::vector<std::pair<Address::ptr, uint32_t>>& result, const std::string& interface_name, int family){
    std::unordered_map<std::string, std::vector<std::pair<Address::ptr, uint32_t>>> results;
    if(!Address::get_interface_address(results, family)){
        return false;
    }
    auto it = results.find(interface_name);
    if(it == results.end())
        return false;
    result = it->second;
    return true;
}
    

template<typename T>
T bit_mask(uint32_t prefix_len){
    if(prefix_len > sizeof(T) * 8)
        prefix_len = sizeof(T) * 8;
    return ~((1ULL << (sizeof(T) * 8 - prefix_len)) - 1);
}

int Address::get_family() const {
    return get_addr()->sa_family;
}

std::string Address::to_string() const{
    std::ostringstream oss;
    output(oss);
    return oss.str();
}

bool Address::operator<(const Address& rhs) const{
    socklen_t min_len = std::min(get_addr_len(), rhs.get_addr_len());
    int result = memcmp(get_addr(), rhs.get_addr(), min_len);
    if(result < 0)
        return true;
    else if(result == 0 && rhs.get_addr_len() > min_len)
        return true;
    return false;
}

bool Address::operator==(const Address& rhs) const{
    socklen_t len = get_addr_len();
    if(len != rhs.get_addr_len())
        return false;
    return memcmp(get_addr(), rhs.get_addr(), len) == 0;
}

bool Address::operator!=(const Address& rhs) const{
    return !(*this == rhs);
}

std::ostream& operator<<(std::ostream& os, const Address& addr){
    return addr.output(os);
}


IPAddress::ptr IPAddress::create(const char* address, uint16_t port){
    if(!address)
        return nullptr;
    IPAddress::ptr rt;

    addrinfo hints, *results;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;

    getaddrinfo(address, nullptr, &hints, &results);
    for(addrinfo* cur = results; cur; cur = cur->ai_next){
        rt = std::dynamic_pointer_cast<IPAddress>(Address::create(cur->ai_addr));
        if(rt){
            rt->set_port(port);
            break;
        }
    }
    freeaddrinfo(results);
    return rt;
}


IPv4Address::IPv4Address(){
    memset(&address_, 0, sizeof(address_));
    address_.sin_family = AF_INET;
}
IPv4Address::IPv4Address(const sockaddr_in& address):
    address_(address){
    ;
}
IPv4Address::IPv4Address(const char* address, uint16_t port){
    memset(&address_, 0, sizeof(address_));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    int ret = inet_pton(AF_INET, address, &address_.sin_addr);
    if(ret <= 0)
        throw std::logic_error("IPv4Address::IPv4Address invalid address");
}
IPv4Address::IPv4Address(uint32_t address, uint16_t port){
    memset(&address_, 0, sizeof(address_));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    address_.sin_addr.s_addr = htonl(address);
}

const sockaddr* IPv4Address::get_addr() const{
    return (const sockaddr*)&address_;
}
sockaddr* IPv4Address::get_addr(){
    return (sockaddr*)&address_;
}

socklen_t IPv4Address::get_addr_len() const{
    return sizeof(address_);
}

std::ostream& IPv4Address::output(std::ostream& os) const{
    uint32_t address = ntohl(address_.sin_addr.s_addr);
    os << ((address >> 24) & 0xFF) << '.'
       << ((address >> 16) & 0xFF) << '.'
       << ((address >> 8) & 0xFF) << '.'
       << (address & 0xFF);
    os << ':' << ntohs(address_.sin_port);
    return os;
}

std::shared_ptr<IPAddress> IPv4Address::get_broadcast_address(uint32_t prefix_len){
    sockaddr_in broadcast_addr(address_);
    if(prefix_len < 32){
        uint32_t mask = bit_mask<uint32_t>(prefix_len);
        broadcast_addr.sin_addr.s_addr |= htonl(~mask);
    }
    return std::make_shared<IPv4Address>(broadcast_addr);
}

std::shared_ptr<IPAddress> IPv4Address::get_network_address(uint32_t prefix_len){
    sockaddr_in network_addr(address_);
    if(prefix_len < 32){
        uint32_t mask = bit_mask<uint32_t>(prefix_len);
        network_addr.sin_addr.s_addr &= htonl(mask);
    }
    return std::make_shared<IPv4Address>(network_addr);
}

uint16_t IPv4Address::get_port() const{
    return ntohs(address_.sin_port);
}
void IPv4Address::set_port(uint16_t port){
    address_.sin_port = htons(port);
}


IPv6Address::IPv6Address(){
    memset(&address_, 0, sizeof(address_));
    address_.sin6_family = AF_INET6;
}
IPv6Address::IPv6Address(const char* address, uint16_t port){
    memset(&address_, 0, sizeof(address_));
    address_.sin6_family = AF_INET6;
    address_.sin6_port = htons(port);
    int ret = inet_pton(AF_INET6, address, &address_.sin6_addr.s6_addr);
    if(ret <= 0)
        throw std::logic_error("IPv6Address::IPv6Address invalid address");
}
IPv6Address::IPv6Address(const sockaddr_in6& address):
    address_(address){
    ;
}
IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port){
    memset(&address_, 0, sizeof(address_));
    address_.sin6_family = AF_INET6;
    address_.sin6_port = htons(port);
    memcpy(&address_.sin6_addr.s6_addr, address, 16);
}

sockaddr* IPv6Address::get_addr(){
    return (sockaddr*)&address_;
}
const sockaddr* IPv6Address::get_addr() const{
    return (const sockaddr*)&address_;
}

socklen_t IPv6Address::get_addr_len() const{
    return sizeof(address_);
}

std::ostream& IPv6Address::output(std::ostream& os) const{
    const uint16_t* address = (const uint16_t*)address_.sin6_addr.s6_addr;
    os << '[' << std::hex << address[0];
    for(int i = 1; i < 8; ++i)
        os << std::hex << ':' << address[i];
    os << ']' << ':' << std::dec << ntohs(address_.sin6_port);
    return os;
}

std::shared_ptr<IPAddress> IPv6Address::get_broadcast_address(uint32_t prefix_len){
    sockaddr_in6 broadcast_addr(address_);
    if(prefix_len < 128){
        broadcast_addr.sin6_addr.s6_addr[prefix_len/8] |= ~bit_mask<uint8_t>(prefix_len % 8);
        for(int i = prefix_len / 8 + 1; i < 16; ++i)
            broadcast_addr.sin6_addr.s6_addr[i] = 0xFF;
    }
    return std::make_shared<IPv6Address>(broadcast_addr);
}

std::shared_ptr<IPAddress> IPv6Address::get_network_address(uint32_t prefix_len){
    sockaddr_in6 network_addr(address_);
    if(prefix_len < 128){
        network_addr.sin6_addr.s6_addr[prefix_len/8] &= bit_mask<uint8_t>(prefix_len % 8);
        for(int i = prefix_len / 8 + 1; i < 16; ++i)
            network_addr.sin6_addr.s6_addr[i] = 0x00;
    }
    return std::make_shared<IPv6Address>(network_addr);
}

uint16_t IPv6Address::get_port() const{
    return ntohs(address_.sin6_port);
}
void IPv6Address::set_port(uint16_t port){
    address_.sin6_port = htons(port);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;
UnixAddress::UnixAddress(){
    memset(&address_, 0, sizeof(address_));
    address_.sun_family = AF_UNIX;
    length_ = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}
UnixAddress::UnixAddress(const std::string& path){
    if(path.length() > MAX_PATH_LEN)
        throw std::logic_error("UnixAddress: path length too long");
    memset(&address_, 0, sizeof(address_));
    address_.sun_family = AF_UNIX;
    strncpy(address_.sun_path, path.c_str(), path.length());
    length_ = offsetof(sockaddr_un, sun_path) + path.length();
}

sockaddr* UnixAddress::get_addr(){
    return (sockaddr*)&address_;
}
const sockaddr* UnixAddress::get_addr() const{
    return (const sockaddr*)&address_;
}

socklen_t UnixAddress::get_addr_len() const{
    return length_;
}

std::string UnixAddress::get_path() const{
    std::string path;
    if(length_ > offsetof(sockaddr_un, sun_path)){
        //abstract socket
        if(address_.sun_path[0] == 0)
            path = "\\0" + std::string(address_.sun_path + 1, length_ - offsetof(sockaddr_un, sun_path) - 1);
        else
            path = address_.sun_path;
    }
    return path;
}

std::ostream& UnixAddress::output(std::ostream& os) const{
    return os << get_path();
}

UnknownAddress::UnknownAddress(){
    memset(&address_, 0, sizeof(address_));
    length_ = sizeof(address_);
}
UnknownAddress::UnknownAddress(int family){
    memset(&address_, 0, sizeof(address_));
    address_.sa_family = family;
    length_ = sizeof(address_);
}
UnknownAddress::UnknownAddress(const sockaddr& address, socklen_t length):
    address_(address),
    length_(length){
    ;
}

sockaddr* UnknownAddress::get_addr(){
    return (sockaddr*)&address_;
}
const sockaddr* UnknownAddress::get_addr() const{
    return (const sockaddr*)&address_;
}

socklen_t UnknownAddress::get_addr_len() const{
    return length_;
}

std::ostream& UnknownAddress::output(std::ostream& os) const{
    os << "UnknownAddress family=" << address_.sa_family
       << " len=" << length_;
    return os;
}

}