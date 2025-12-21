#include "hook.h"
#include "log.h"
#include "iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

void test_sleep() {
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(2, "iomanager_test"));

    iom->add_fiber([](void*){
        sleep(2);
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sleep 2";
    });

    iom->add_fiber([](void*){
        sleep(3);
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sleep 3";
    });
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "test_sleep";
}

void test_hook(void*){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.74.102", &addr.sin_addr.s_addr);

    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.1\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << buff;
}

int main(){
    //test_sleep();
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(1, "iomanager_test"));
    iom->add_fiber(test_hook, nullptr);
    return 0;
}