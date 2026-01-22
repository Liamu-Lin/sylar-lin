#include "tcp_server.h"
#include "bytearry.h"

#include <vector>
#include <iostream>

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

class EchoServer : public sylar::TcpServer{
public:

private:
    void handle_client(sylar::Socket::ptr client, void*) override{
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "handle_client " << *client;
        sylar::ByteArray::ptr ba(new sylar::ByteArray);
        char buf[4096];
        while(true){
            ba->clear();
            int rt = client->recv(buf, sizeof(buf));
            if(rt == 0){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "client close: " << *client;
                break;
            }else if(rt < 0){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR) << "client error rt=" << rt
                    << " errno=" << errno << " errstr=" << strerror(errno)
                    << " client=" << *client;
                break;
            }
            ba->write(buf, rt);
            ba->read(buf, rt);
            std::cout.write(buf, rt);
            client->send(buf, rt);
        }
    }
};

void run(void*){
    EchoServer::ptr es(new EchoServer);
    sylar::Address::ptr addr = sylar::Address::lookup_address("127.0.0.1:8020");
    es->bind(addr);
    es->start();
}

int main(){
    sylar::IOManager iom(2);
    iom.add_fiber(run);
    iom.start();

    return 0;
}