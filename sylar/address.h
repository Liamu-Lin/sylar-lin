#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include "log.h"

#include <memory>
#include <vector>
#include <string>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>

namespace sylar{

class Address{
public:
/*
    static std::shared_ptr<Address> create(const sockaddr* addr, socklen_t addr_len);
    // return all addresses which match the conditions
    // ipv6 -- [host]:port/service
    // ipv4 -- host:port/service
    static bool get_host_addresses(std::vector<std::shared_ptr<Address>>& result, const std::string& host, 
                                   int family = AF_INET, int socket_type = 0, int protocol = 0);
*/

    virtual ~Address() {}

    int get_family() const;
    virtual const sockaddr* get_addr() const = 0;
    virtual socklen_t get_addr_len() const = 0;
    virtual std::ostream& output(std::ostream& os) const = 0;
    std::string to_string() const;

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
protected:
    virtual sockaddr* get_addr() = 0;
};

class IPAddress : public Address{
public:
    virtual ~IPAddress() {}
    // get broadcast address, address | ~mask: XXXXFF
    virtual std::shared_ptr<IPAddress> get_broadcast_address(uint32_t prefix_len) = 0;
    // get network address, address & mask: XXXX00
    virtual std::shared_ptr<IPAddress> get_network_address(uint32_t prefix_len) = 0;
    virtual uint32_t get_port() const = 0;
    virtual void set_port(uint32_t port) = 0;
};

class IPv4Address : public IPAddress{
public:
    IPv4Address();
    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;

    std::shared_ptr<IPAddress> get_broadcast_address(uint32_t prefix_len) override;
    std::shared_ptr<IPAddress> get_network_address(uint32_t prefix_len) override;
    uint32_t get_port() const override;
    void set_port(uint32_t port) override;
private:
    sockaddr* get_addr() override;
private:
    sockaddr_in address_;
};

class IPv6Address : public IPAddress{
public:
    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint32_t port = 0);

    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;

    std::shared_ptr<IPAddress> get_broadcast_address(uint32_t prefix_len) override;
    std::shared_ptr<IPAddress> get_network_address(uint32_t prefix_len) override;
    uint32_t get_port() const override;
    void set_port(uint32_t port) override;
private:
    sockaddr* get_addr() override;
private:
    sockaddr_in6 address_;
};

class UnixAddress : public Address{
public:
    UnixAddress();
    UnixAddress(const std::string& path);

    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;
private:
    sockaddr* get_addr() override;
private:
    sockaddr_un address_;
    socklen_t length_;
};

class UnknownAddress : public Address{
public:
    UnknownAddress();
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& address, socklen_t length);

    const sockaddr* get_addr() const override;
    socklen_t get_addr_len() const override;
    std::ostream& output(std::ostream& os) const override;
private:
    sockaddr* get_addr() override;
private:
    sockaddr address_;
    socklen_t length_;
};

}


#endif