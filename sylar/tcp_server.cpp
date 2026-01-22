#include "tcp_server.h"
#include "config.h"
#include "fdmanager.h"

namespace sylar{

Logger::ptr g_logger = LoggerMgr.get_logger("system");

ConfigVar<uint64_t>::ptr g_tcp_server_recv_timeout = 
            ConfigMgr.look_up<uint64_t>("tcp.server.recv_timeout", 60 * 1000, "tcp server recv timeout");

TcpServer::TcpServer(IOManager* worker, IOManager* accept_worker):
    worker_(worker),
    accept_worker_(accept_worker),
    recv_timeout_(g_tcp_server_recv_timeout->get_value()),
    is_stop_(true){
    ;
}

TcpServer::~TcpServer(){
    stop();
}

bool TcpServer::bind(Address::ptr addr){
    std::vector<Address::ptr> addrs;
    addrs.push_back(addr);
    return bind(addrs, nullptr);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>* fails){
    bool rt = true;
    for(auto& addr : addrs){
        Socket::ptr sock(std::make_shared<Socket>(addr->get_family(), Socket::TCP));
        if(!sock->bind(addr)){
            SYLAR_LOG(g_logger, LogLevel::Level::ERROR) << "bind fail errno=" << errno
                << " errstr=" << strerror(errno) << " addr=" << addr->to_string();
            if(fails)
                fails->push_back(addr);
            rt = false;
            continue;
        }
        if(!sock->listen()){
            SYLAR_LOG(g_logger, LogLevel::Level::ERROR) << "listen fail errno=" << errno
                << " errstr=" << strerror(errno) << " addr=" << addr->to_string();
            if(fails)
                fails->push_back(addr);
            rt = false;
            continue;
        }
        listen_socks_.push_back(sock);
        SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "bind success addr=" << *addr;
    }
    return rt;
}

bool TcpServer::start(){
    if(!is_stop_)
        return true;
    for(auto& sock : listen_socks_){
        accept_worker_->add_fiber(std::bind(&TcpServer::on_accepted, shared_from_this(), sock, std::placeholders::_1));
    }
    is_stop_ = false;
    return true;
}

bool TcpServer::stop(){
    if(is_stop_)
        return true;
    for(auto& sock : listen_socks_){
        sock->cancel_all();
        sock->close();
    }
    listen_socks_.clear();
    is_stop_ = true;
    return true;
}

void TcpServer::on_accepted(Socket::ptr sock, void*){
    while(!is_stop_){
        Socket::ptr client = sock->accept();
        if(client){
            SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "accept success from " << *client->get_remote_address()
                << " to " << *client->get_local_address();
            client->set_timeout(SO_RCVTIMEO, recv_timeout_);
            worker_->add_fiber(std::bind(&TcpServer::handle_client, shared_from_this(), client, std::placeholders::_1));
        }else{
            SYLAR_LOG(g_logger, LogLevel::Level::ERROR) << "accept fail errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
    SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "TcpServer::on_accepted exit";
}

void TcpServer::handle_client(Socket::ptr client, void*){
    SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "handle_client " << *client;
    char* buf = new char[1024];
    memset(buf, 0, 1024);
    client->recv(buf, 1024);
    SYLAR_LOG(g_logger, LogLevel::Level::INFO) << "recv " << buf;
    delete[] buf;
    client->close();
}


}