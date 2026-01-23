#ifndef __SYLAR_STREAM_H__
#define __SYLAR_STREAM_H__

#include "bytearry.h"
#include "socket.h"
#include <memory>

namespace sylar{

class Stream{
public:
    typedef std::shared_ptr<Stream> ptr;
public:
    virtual ~Stream() = default;

    virtual size_t read(void* buffer, size_t length) = 0;
    virtual size_t read(ByteArray::ptr ba, size_t length) = 0;
    virtual void read_fix_length(void* buffer, size_t length);
    virtual void read_fix_length(ByteArray::ptr ba, size_t length);

    virtual size_t write(const void* buffer, size_t length) = 0;
    virtual size_t write(ByteArray::ptr ba, size_t length) = 0;
    virtual void write_fix_length(const void* buffer, size_t length);
    virtual void write_fix_length(ByteArray::ptr ba, size_t length);
};

class SocketStream : public Stream{
public:
    typedef std::shared_ptr<SocketStream> ptr;
public:
    SocketStream(Socket::ptr sock);
    ~SocketStream() override;

    size_t read(void* buffer, size_t length) override;
    size_t read(ByteArray::ptr ba, size_t length) override;

    size_t write(const void* buffer, size_t length) override;
    size_t write(ByteArray::ptr ba, size_t length) override;
private:
    Socket::ptr socket_;
};


}

#endif