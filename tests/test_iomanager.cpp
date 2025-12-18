#include "iomanager.h"
#include "thread.h"
#include "log.h"

#include <iostream>

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct arg_struct{
    int a;
    std::string b;
};
int sock;
void func(void* arg){
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "59.82.122.140", &addr.sin_addr.s_addr);

    //int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))){
        ;
    }
    else if(errno == EINPROGRESS) {
        SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
            << "sock connecting...";
        iom->add_event(sock, EPOLLOUT, [](void*){
            SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
                << "sock connected!";
            //close(sock);
            const char* req = "GET / HTTP/1.1\r\nHost: 59.82.122.140\r\nConnection: close\r\n\r\n";
            send(sock, req, strlen(req), 0);
        });
        iom->add_event(sock, EPOLLIN, [](void*){
            SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
                << "sock recv data!";
            close(sock);
        });
    }
    else {
        SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
            << "sock connect error errno=" << errno;
    }
    
}

void test_iomanager(){
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(2, "iomanager_test"));
    iom->add_fiber(func);
}

int main(){

    test_iomanager();
    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "IOManager test finished.";

    return 0;
}