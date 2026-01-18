#ifndef __SYLAR_BYTEARRY_H__
#define __SYLAR_BYTEARRY_H__

#include <memory>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <sys/uio.h>
namespace sylar{

class ByteArray{
public:
    typedef std::shared_ptr<ByteArray> ptr;
    struct MemoryBlock{
        MemoryBlock(size_t block_size);
        ~MemoryBlock();
        uint8_t* data;
        size_t size;
        MemoryBlock* next;
    };
public:
    ByteArray(size_t block_size = 4096);
    ~ByteArray();

    // write fixed length integers
    void write_fix_b(uint8_t value);
    void write_fix_s(uint16_t value);
    void write_fix_l(uint32_t value);
    void write_fix_ll(uint64_t value);
    // write variable length integers
    void write_var_l(int32_t value);
    void write_var_ul(uint32_t value);
    void write_var_ll(int64_t value);
    void write_var_ull(uint64_t value);
    // write float/double
    void write_float(float value);
    void write_double(double value);
    // write string
    void write_string(const std::string& value);
    void write_string_with_length(const std::string& value);

    // read fixed length integers
    int8_t read_fix_b();
    uint8_t read_fix_ub();
    int16_t read_fix_s();
    uint16_t read_fix_us();
    int32_t read_fix_l();
    uint32_t read_fix_ul();
    int64_t read_fix_ll();
    uint64_t read_fix_ull();
    // read variable length integers
    int32_t read_var_l();
    uint32_t read_var_ul();
    int64_t read_var_ll();
    uint64_t read_var_ull();
    // read float/double
    float read_float();
    double read_double();
    // read string
    // read string, whose length is prefixed with size_t
    std::string read_string();
    // read string with fixed length
    std::string read_string_with_length(size_t length);

    void write(const void* data, size_t size);
    size_t read(void* buf, size_t size);

    void write_to_file(const std::string& filename);
    void read_from_file(const std::string& filename);

    void set_read_position(size_t pos);
    size_t get_read_position() const { return read_pos_; }
    size_t get_write_position() const { return write_pos_; }
    size_t get_size() const { return write_pos_; }
    size_t get_unread_size() const { return write_pos_ - read_pos_; }
    size_t get_capacity() const { return capacity_; }
    size_t get_spare_capacity() const { return capacity_ - write_pos_; }

    void get_read_buffers(std::vector<iovec>& buffers, size_t length) const;
    void get_write_buffers(std::vector<iovec>& buffers, size_t length);

    std::string to_hex_string();
    void clear();
private:
    // ensure there is enough space to write size+1 bytes
    void expand_capacity(size_t size);
private:
    size_t block_size_;
    size_t capacity_;
    MemoryBlock* root_;

    size_t read_pos_;
    size_t write_pos_;
    MemoryBlock* read_cur_;
    MemoryBlock* write_cur_;
};

}

#endif