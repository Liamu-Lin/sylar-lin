#include "iomanager.h"
#include "thread.h"
#include "log.h"

#include <iostream>

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

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
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "sock connecting...";
        iom->add_event(sock, EPOLLOUT, [](void*){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sock connected!";
            //close(sock);
            const char* req = "GET / HTTP/1.1\r\nHost: 59.82.122.140\r\nConnection: close\r\n\r\n";
            send(sock, req, strlen(req), 0);
        });
        iom->add_event(sock, EPOLLIN, [](void*){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sock recv data!";
            close(sock);
        });
    }
    else {
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "sock connect error errno=" << errno;
    }
    
}

void test_iomanager(sylar::IOManager* iom){
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "test_iomanager";
    iom->add_fiber(func);
}

sylar::Timer::ptr s_timer;
void test_timer(sylar::IOManager* iom){
    s_timer = iom->add_timer(true, 1000, [](void*){
        static int i = 0;
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "hello timer i=" << i;
        if(++i == 3) {
            //s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, nullptr);
    SYLAR_ASSERT(s_timer);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) 
        << " args=" << s_timer->get_args();
}

int main(){
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(2, "iomanager_test"));

    test_iomanager(iom.get());

    test_timer(iom.get());
    

    return 0;
}