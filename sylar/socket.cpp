#include "socket.h"
#include "fdmanager.h"
#include "iomanager.h"

namespace sylar{

static Logger::ptr g_logger = LoggerMgr.get_logger("system");

Socket::Socket(int family, int type, int protocol):
    sockfd_(-1),
    family_(family),
    type_(type),
    protocol_(protocol),
    is_connected_(false){
    ;
}
Socket::~Socket(){
    close();
}

bool Socket::is_valid() const{
    return sockfd_ != -1;
}
bool Socket::is_connected() const{
    return is_connected_;
}
int Socket::get_family() const{
    return family_;
}
int Socket::get_type() const{
    return type_;
}
int Socket::get_protocol() const{
    return protocol_;
}
int Socket::get_fd() const{
    return sockfd_;
}
int Socket::get_error() const{
    int error = 0;
    socklen_t len = sizeof(error);
    if(!get_option(SOL_SOCKET, SO_ERROR, &error, &len))
        error = errno;
    return error;
}
uint64_t Socket::get_timeout(int type) const{
    timeval tv;
    get_option(SOL_SOCKET, type, tv);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

Address::ptr Socket::get_local_address() const{
    if(sockfd_ == -1)
        return nullptr;
    if(local_address_)
        return local_address_;
    local_address_ = Address::create(family_);
    socklen_t addrlen = local_address_->get_addr_len();
    if(getsockname(sockfd_, local_address_->get_addr(), &addrlen)){
        local_address_.reset();
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::get_local_address getsockname error"
            << " errno=" << errno << " errstr=" << strerror(errno);
    }
    return local_address_;
}

Address::ptr Socket::get_remote_address() const{
    if(sockfd_ == -1 || !is_connected_)
        return nullptr;
    if(remote_address_)
        return remote_address_;
    remote_address_ = Address::create(family_);
    socklen_t addrlen = remote_address_->get_addr_len();
    if(getpeername(sockfd_, remote_address_->get_addr(), &addrlen)){
        remote_address_.reset();
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::get_remote_address getpeername error"
            << " errno=" << errno << " errstr=" << strerror(errno);
    }
    return remote_address_;
}

bool Socket::get_option(int level, int option, void* result, socklen_t* len) const{
    int rt = getsockopt(sockfd_, level, option, result, len);
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::get_option getsockopt error"
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

void Socket::set_timeout(int type, uint64_t value){
    timeval tv;
    tv.tv_sec = value / 1000;
    tv.tv_usec = (value % 1000) * 1000;
    set_option(SOL_SOCKET, type, tv);
}

bool Socket::set_option(int level, int option, const void* result, socklen_t len){
    int rt = setsockopt(sockfd_, level, option, result, len);
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::set_option setsockopt error"
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept(){
    Socket::ptr comm_sock(new Socket(family_, type_, protocol_));
    comm_sock->remote_address_ = Address::create(family_);
    socklen_t addrlen = comm_sock->remote_address_->get_addr_len();
    int comm_sockfd = ::accept(sockfd_, comm_sock->remote_address_->get_addr(), &addrlen);
    if(comm_sockfd == -1){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::accept accept error"
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    comm_sock->init(comm_sockfd);
    return comm_sock;
}

bool Socket::init(int sockfd){
    if(sockfd == -1){
        sockfd_ = ::socket(family_, type_, protocol_);
        is_connected_ = false;
    }
    else{
        sockfd_ = sockfd;
        is_connected_ = true;
    }
    if(sockfd_ == -1){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::init socket error"
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    // if syscall is hooked, set the socket to non-blocking and close-on-exec
    if(FdMgr.get_fdctx(sockfd_)){
        set_option(SOL_SOCKET, SO_REUSEADDR, 1);
        if(type_ == SOCK_STREAM)
            set_option(IPPROTO_TCP, TCP_NODELAY, 1);
    }
    return true;
}

bool Socket::bind(const Address::ptr addr){
    if(!is_valid())
        init(-1);
    if(!is_valid() || addr->get_family() != family_){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::bind invalid socket or address family not match"
            << " sock=" << to_string()
            << " addr=" << addr->to_string();
        return false;
    }

    int rt = ::bind(sockfd_, addr->get_addr(), addr->get_addr_len());
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::bind bind error"
            << " sock=" << to_string()
            << " addr=" << addr->to_string()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::listen(int backlog){
    if(!is_valid()){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::listen invalid socket"
            << " sock=" << to_string();
        return false;
    }
    int rt = ::listen(sockfd_, backlog);
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::listen listen error"
            << " sock=" << to_string()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::connect(const Address::ptr addr){
    if(!is_valid())
        init(-1);
    if(!is_valid() || !addr || addr->get_family() != family_){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::connect  invalid socket or address"
            << " sock=" << to_string()
            << " addr=" << addr->to_string();
        return false;
    }

    int rt = ::connect(sockfd_, addr->get_addr(), addr->get_addr_len());
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::connect connect error"
            << " sock=" << to_string()
            << " addr=" << addr->to_string()
            << " errno=" << errno << " errstr=" << strerror(errno);
        close();
        return false;
    }
    is_connected_ = true;
    return true;
}

bool Socket::reconnect(uint64_t timeout_ms){
    if(get_remote_address() == nullptr)
        return false;
    local_address_.reset();
    return connect(get_remote_address());
}

bool Socket::close(){
    is_connected_ = false;
    if(sockfd_ == -1)
        return true;
    int rt = ::close(sockfd_);
    if(rt){
        SYLAR_LOG(g_logger, LogLevel::Level::ERROR)
            << "Socket::close close error"
            << " sock=" << to_string()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

int Socket::send(const void* buffer, size_t length, int flags){
    if(!is_connected_)
        return -1;
    return ::send(sockfd_, buffer, length, flags);
}
int Socket::send(const iovec* buffers, size_t length, int flags){
    if(!is_connected_)
        return -1;
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::sendmsg(sockfd_, &msg, flags);
}
int Socket::send_to(const void* buffer, size_t length, const Address::ptr to, int flags){
    if(!to || !!is_valid())
        return -1;
    return ::sendto(sockfd_, buffer, length, flags, to->get_addr(), to->get_addr_len());
}
int Socket::send_to(const iovec* buffers, size_t length, const Address::ptr to, int flags){
    if(!to || !is_valid())
        return -1;
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->get_addr();
    msg.msg_namelen = to->get_addr_len();
    return ::sendmsg(sockfd_, &msg, flags);
}

int Socket::recv(void* buffer, size_t length, int flags){
    if(!is_connected_)
        return -1;
    return ::recv(sockfd_, buffer, length, flags);
}
int Socket::recv(iovec* buffers, size_t length, int flags){
    if(!is_connected_)
        return -1;
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    return ::recvmsg(sockfd_, &msg, flags);
}
int Socket::recv_from(void* buffer, size_t length, Address::ptr from, int flags){
    if(!is_valid() || !from)
        return -1;
    sockaddr* addr = nullptr;
    socklen_t addrlen = 0;
    addr = from->get_addr();
    addrlen = from->get_addr_len();
    return ::recvfrom(sockfd_, buffer, length, flags, addr, &addrlen);
}
int Socket::recv_from(iovec* buffers, size_t length, Address::ptr from, int flags){
    if(!is_valid() || !from)
        return -1;
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    sockaddr* addr = nullptr;
    socklen_t addrlen = 0;
    addr = from->get_addr();
    addrlen = from->get_addr_len();
    msg.msg_name = addr;
    msg.msg_namelen = addrlen;
    return ::recvmsg(sockfd_, &msg, flags);
}

std::ostream& operator<<(std::ostream& os, const Socket& sock){
    os << sock.to_string();
    return os;
}

std::string Socket::to_string() const{
    std::ostringstream oss;
    oss << "[Socket sock=" << sockfd_
        << " family=" << family_
        << " type=" << type_
        << " protocol=" << protocol_
        << " local_address=";
    const Address::ptr local = get_local_address();
    if(local)
        oss << local->to_string();
    else
        oss << "nullptr";
    oss << " remote_address=";
    const Address::ptr remote = get_remote_address();
    if(remote)
        oss << remote->to_string();
    else
        oss << "nullptr";
    oss << " is_connected=" << is_connected_ << "]";
    return oss.str();
}

bool Socket::cancel_read(){
    return IOManager::GetThis()->cancel_event(sockfd_, EPOLLIN);
}
bool Socket::cancel_write(){
    return IOManager::GetThis()->cancel_event(sockfd_, EPOLLOUT);
}
bool Socket::cancel_accept(){
    return IOManager::GetThis()->cancel_event(sockfd_, EPOLLIN);
}
bool Socket::cancel_all(){
    return IOManager::GetThis()->cancel_all(sockfd_);
}


}