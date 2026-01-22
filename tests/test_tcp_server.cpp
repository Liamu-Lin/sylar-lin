#include "tcp_server.h"
#include "iomanager.h"
#include <vector>
void test_tcp_server(void* arg = nullptr){
    sylar::TcpServer::ptr server(new sylar::TcpServer);
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8356");
    //sylar::Address::ptr unix_addr = sylar::Address::ptr(new sylar::UnixAddress("/tmp/sylar.sock"));
    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(unix_addr);
    server->bind(addrs);
    server->start();
    sleep(5);
    server->stop();
}

int main(){
    sylar::IOManager iom(5);

    test_tcp_server();

    return 0;
}