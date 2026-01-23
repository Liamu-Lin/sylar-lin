#include "stream.h"
#include <stdexcept>

namespace sylar{

void Stream::read_fix_length(void* buffer, size_t length){
    size_t offset = 0;
    while(offset < length){
        try{
            size_t len = read((char*)buffer + offset, length - offset);
            if(len <= 0)
                throw std::runtime_error("Stream::read_fix_length failed, read length=" + std::to_string(offset));
            offset += len;
        } catch(...) {
            throw;
        }
    }
}
void Stream::read_fix_length(ByteArray::ptr ba, size_t length){
    size_t offset = 0;
    while(offset < length){
        try{
            size_t len = read(ba, length - offset);
            if(len <= 0)
                throw std::runtime_error("Stream::read_fix_length failed, read length=" + std::to_string(offset));
            offset += len;
        } catch(...) {
            throw;
        }
    }
}

void Stream::write_fix_length(const void* buffer, size_t length){
    size_t offset = 0;
    while(offset < length){
        try{
            size_t len = write((const char*)buffer + offset, length - offset);
            if(len <= 0)
                throw std::runtime_error("Stream::write_fix_length failed, written length=" + std::to_string(offset));
            offset += len;
        } catch(...) {
            throw;
        }
    }
}
void Stream::write_fix_length(ByteArray::ptr ba, size_t length){
    size_t offset = 0;
    while(offset < length){
        try{
            size_t len = write(ba, length - offset);
            if(len <= 0)
                throw std::runtime_error("Stream::write_fix_length failed, written length=" + std::to_string(offset));
            offset += len;
        } catch(...) {
            throw;
        }
    }
}


SocketStream::SocketStream(Socket::ptr sock)
    :socket_(sock) {
}
SocketStream::~SocketStream() {
}

size_t SocketStream::read(void* buffer, size_t length){
    if(!socket_ || !socket_->is_connected())
        throw std::logic_error("SocketStream::read socket is null");
    int ret = socket_->recv(buffer, length);
    if(ret == 0)
        throw std::runtime_error("SocketStream::read socket closed or EOF");
    else if(ret < 0)
        throw std::runtime_error("SocketStream::read failed");
    return ret;
}
size_t SocketStream::read(ByteArray::ptr ba, size_t length){
    if(!socket_ || !socket_->is_connected())
        throw std::logic_error("SocketStream::read socket is null");
    std::vector<iovec> iovs;
    ba->get_write_buffers(iovs, length);
    int ret = socket_->recv(iovs.data(), iovs.size());
    if(ret <= 0)
        throw std::runtime_error("SocketStream::read failed");
    ba->set_write_position(ba->get_write_position() + ret);
    return ret;
}

size_t SocketStream::write(const void* buffer, size_t length){
    if(!socket_ || !socket_->is_connected())
        throw std::logic_error("SocketStream::write socket is null");
    int ret = socket_->send(buffer, length);
    if(ret <= 0)
        throw std::runtime_error("SocketStream::write failed");
    return ret;
}
size_t SocketStream::write(ByteArray::ptr ba, size_t length){
    if(!socket_ || !socket_->is_connected())
        throw std::logic_error("SocketStream::write socket is null");
    std::vector<iovec> iovs;
    ba->get_read_buffers(iovs, length);
    int ret = socket_->send(iovs.data(), iovs.size());
    if(ret <= 0)
        throw std::runtime_error("SocketStream::write failed");
    ba->set_read_position(ba->get_read_position() + ret);
    return ret;
}


}