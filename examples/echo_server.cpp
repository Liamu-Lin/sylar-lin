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
        while(is_stop_ == false){
            std::vector<iovec> iovs;
            ba->get_write_buffers(iovs, 4096);
            int rt = client->recv(&iovs[0], iovs.size());
            if(rt <= 0){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "client close " << *client;
                break;
            }
            ba->set_write_position(ba->get_write_position() + rt);
            std::string str;
            str.resize(rt+1);
            ba->read(&str[0], rt);
            std::cout << str;
        }
        ba->set_read_position(0);
        std::cout << ba->to_hex_string() << std::endl;
    }
};

void run(void*){
    EchoServer::ptr es(new EchoServer);
    sylar::Address::ptr addr = sylar::Address::lookup_address("0.0.0.0:8020");
    es->bind(addr);
    es->start();
}

int main(){
    sylar::IOManager iom(2);
    iom.add_fiber(run);
    iom.start();

    return 0;
}