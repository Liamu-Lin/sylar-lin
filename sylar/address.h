#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include "log.h"

#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

namespace sylar{

class Address{
public:
    typedef std::shared_ptr<Address> ptr;
    static Address::ptr create(int family);
    static Address::ptr create(const sockaddr* addr);
    // return all addresses which match the conditions
    // ipv6 -- [host]:port/service
    // ipv4 -- host:port/service
    static bool lookup_address(std::vector<Address::ptr>& result, const std::string& host, 
                                   int family = AF_INET, int socket_type = 0, int protocol = 0);
    static Address::ptr lookup_address(const std::string& host, int family = AF_INET,
                                                   int socket_type = 0, int protocol = 0);

    // get all interface addresses
    // <interface_name, <address, prefix_len>...>
    static bool get_interface_address(std::unordered_map<std::string, std::vector<std::pair<Address::ptr, uint32_t>>>& result,
                                      int family = AF_INET);
    static bool get_interface_address(std::vector<std::pair<Address::ptr, uint32_t>>& result,
                                      const std::string& interface_name, int family = AF_INET);

    virtual ~Address() {}

    int get_family() const;
    virtual sockaddr* get_addr() = 0;
    virtual const sockaddr* get_addr() const = 0;
    virtual socklen_t get_addr_len() const = 0;
    virtual std::ostream& output(std::ostream& os) const = 0;
    std::string to_string() const;

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};
std::ostream& operator<<(std::ostream& os, const Address& addr);

class IPAddress : public Address{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    static IPAddress::ptr create(const char* address, uint16_t port = 0);
    // return ip addresses which match the conditions
    // ipv6 -- [host]:port/service
    // ipv4 -- host:port/service
    static bool lookup_ip_address(std::vector<IPAddress::ptr>& result, const std::string& host, int socket_type = 0, int protocol = 0);
    static IPAddress::ptr lookup_ip_address(const std::string& host, int socket_type = 0, int protocol = 0);

    virtual ~IPAddress() {}
    // get broadcast address, address | ~mask: XXXXFF
    virtual IPAddress::ptr get_broadcast_address(uint32_t prefix_len) = 0;
    // get network address, address & mask: XXXX00
    virtual IPAddress::ptr get_network_address(uint32_t prefix_len) = 0;
    virtual uint16_t get_port() const = 0;
    virtual void set_port(uint16_t port) = 0;
};

class IPv4Address : public IPAddress{
public:
    IPv4Address();
    IPv4Address(const sockaddr_in& address);
    IPv4Address(const char* address, uint16_t port = 0);
    IPv4Address(uint32_t address, uint16_t port = 0);

    sockaddr* get_addr() override;
    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;

    IPAddress::ptr get_broadcast_address(uint32_t prefix_len) override;
    IPAddress::ptr get_network_address(uint32_t prefix_len) override;
    uint16_t get_port() const override;
    void set_port(uint16_t port) override;
private:
    sockaddr_in address_;
};

class IPv6Address : public IPAddress{
public:
    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const char* address, uint16_t port = 0);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    sockaddr* get_addr() override;
    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;

    IPAddress::ptr get_broadcast_address(uint32_t prefix_len) override;
    IPAddress::ptr get_network_address(uint32_t prefix_len) override;
    uint16_t get_port() const override;
    void set_port(uint16_t port) override;
private:
    sockaddr_in6 address_;
};

class UnixAddress : public Address{
public:
    UnixAddress();
    UnixAddress(const std::string& path);

    sockaddr* get_addr() override;
    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::string get_path() const;
    std::ostream& output(std::ostream& os) const override;
private:
    sockaddr_un address_;
    socklen_t length_;
};

class UnknownAddress : public Address{
public:
    UnknownAddress();
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& address, socklen_t length);

    sockaddr* get_addr() override;
    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;
private:
    sockaddr address_;
    socklen_t length_;
};

}


#endif