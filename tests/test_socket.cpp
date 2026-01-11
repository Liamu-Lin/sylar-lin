#include "log.h"
#include "address.h"
#include "socket.h"
#include "iomanager.h"

#include <string>
#include <vector>

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("test");

struct test_args{
    const char* host;
    uint16_t port;
};
void test_socket(void* arg){
    test_args* args = (test_args*)arg;
    std::string host = args->host;
    uint16_t port = args->port;

    auto addr = sylar::Address::lookup_address(host + ":" + std::to_string(port));
    if(addr){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "lookup " << host << " success: " << addr->to_string();
    }
    else{
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR)
            << "lookup " << host << " failed";
        return;
    }

    sylar::Socket::ptr sock = sylar::Socket::ptr(new sylar::Socket(addr->get_family(), sylar::Socket::Type::TCP));

    bool rt = sock->connect(addr);
    if(!rt){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR)
            << "connect " << addr->to_string() << " failed";
        return;
    }
    else{
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "connect " << addr->to_string() << " success";
    }
    
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    rt = sock->send(buff, sizeof(buff));
    if(!rt){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR)
            << "send failed";
        return;
    }
    else{
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "send success";
    }

    std::string response;
    response.resize(4096);
    rt = sock->recv(&response[0], response.size());
    if(!rt){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::ERROR)
            << "recv failed";
        return;
    }
    else{
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "recv success rt=" << rt
            << " response=" << response.substr(0, 20);
    }


}

int main(){
    
    sylar::IOManager iom(2);

    std::vector<test_args> arg_vec;
    arg_vec.emplace_back(test_args{"www.baidu.com", 80});
    arg_vec.emplace_back(test_args{"www.huawei.com", 80});
    arg_vec.emplace_back(test_args{"www.taobao.com", 80});
    arg_vec.emplace_back(test_args{"www.tencent.com", 80});
    arg_vec.emplace_back(test_args{"www.sina.com.cn", 80});
    
    for(size_t i = 0; i < arg_vec.size(); ++i){
        iom.add_fiber(test_socket, &arg_vec[i]);
    }

    return 0;
}