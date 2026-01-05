#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "log.h"
#include "address.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

void test_lookup(){
    std::vector<sylar::Address::ptr> addrs;
    bool v = sylar::Address::lookup_address(addrs, "www.baidu.com", AF_INET, SOCK_STREAM);
    if(!v){
        std::cout << "lookup failed" << std::endl;
        return;
    }
    for(auto& addr : addrs)
        std::cout << addr->to_string() << std::endl;
}

void test_iface(){
    std::unordered_map<std::string, std::vector<std::pair<sylar::Address::ptr, uint32_t>>> results;
    bool v = sylar::Address::get_interface_address(results, AF_INET);
    if(!v){
        std::cout << "get_interface_address failed" << std::endl;
        return;
    }
    for(auto& item : results){
        std::cout << item.first << " : " << std::endl;
        for(auto& addr_pair : item.second){
            std::cout << "    " << addr_pair.first->to_string()
                      << " / " << addr_pair.second << std::endl;
        }
    }
}

void test_create(){
    std::vector<sylar::IPAddress::ptr> addrs;
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test create address 1";


    addrs.emplace_back(sylar::IPAddress::create("www.baidu.com", 80));
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test create address 2";

    try{
        addrs.emplace_back(std::make_shared<sylar::IPv4Address>("180.101.49.44", 80));
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test create address 3";
    }catch(...){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR) << "create IPv4Address failed";
    }
    
    try{
        addrs.emplace_back(std::make_shared<sylar::IPv6Address>("824:3d87:2200:cb18:0:ff00:37b0:d8e6", 80));
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test create address end";
    }catch(...){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR) << "create IPv6Address failed";
    }

    for(auto& addr : addrs){
        if(addr)
            std::cout << addr->to_string() << std::endl;
        else
            std::cout << "addr is null" << std::endl;
    }

}

int main(){
    test_lookup();
    test_iface();
    test_create();
    return 0;
}