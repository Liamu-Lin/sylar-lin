#include "stream.h"
#include "log.h"
#include <stdexcept>

namespace sylar{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

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
    socket_->close();
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

bool SocketStream::read_chunked_body(std::string& body, char* buffer, uint64_t buff_size, size_t& remained){
    SYLAR_LOG(g_logger, LogLevel::Level::DEBUG) << "SocketStream::read_chunked_body remained=" << remained;
    size_t chunk_size = 0, tmp_size = 0;
    bool read_chunk_size = false;
    size_t parsed = 0, read_len = remained;
    while(true){
        if(parsed >= read_len){
            try{
                read_len = read(&buffer[0], buff_size);
                parsed = 0;
            } catch(std::runtime_error& e){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "SocketStream::read_chunked_body read error " << e.what();
                return false;
            } catch(...){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "SocketStream::read_chunked_body read error";
                return false;
            }
        }
        if(chunk_size == 0){
            char c = std::toupper(buffer[parsed]);
            parsed += 1;
            if(c >= '0' && c <= '9'){
                tmp_size = (tmp_size << 4) + (c - '0');
                read_chunk_size = true;
            }
            else if(c >= 'A' && c <= 'F'){
                tmp_size = (tmp_size << 4) + (c - 'A' + 10);
                read_chunk_size = true;
            }
            else if(read_chunk_size){
                parsed += 1; // skip \n
                if(tmp_size == 0)
                    break;
                chunk_size = tmp_size;
                tmp_size = 0;
                read_chunk_size = false;
            }
        }
        else if(chunk_size <= read_len - parsed){
            body.append(&buffer[parsed], chunk_size);
            parsed += chunk_size;
            chunk_size = 0;
        }
        else{
            body.append(&buffer[parsed], read_len - parsed);
            chunk_size -= (read_len - parsed);
            parsed = read_len;
        }
    }
    remained = read_len - parsed;
    return true;
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