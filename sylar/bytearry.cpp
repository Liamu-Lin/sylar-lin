#include "bytearry.h"
#include "util.h"
#include <arpa/inet.h>
#include <sstream>

#include <iostream>
#include <iomanip>

namespace sylar{

ByteArray::MemoryBlock::MemoryBlock(size_t block_size):
    data(new uint8_t[block_size]),
    size(block_size),
    next(nullptr){
}
ByteArray::MemoryBlock::~MemoryBlock(){
    delete[] data;
}


ByteArray::ByteArray(size_t block_size):
    block_size_(block_size),
    capacity_(0),
    root_(nullptr),
    read_pos_(0),
    write_pos_(0),
    read_cur_(nullptr),
    write_cur_(nullptr){
}
ByteArray::~ByteArray(){
    clear();
}

void ByteArray::write(const void* data, size_t size){
    if(size == 0)
        return;
    expand_capacity(size);
    size_t data_idx = 0;
    const uint8_t* src = static_cast<const uint8_t*>(data);
    while(data_idx < size){
        size_t block_pos = write_pos_ % block_size_;
        size_t block_space = block_size_ - block_pos;
        size_t write_size = std::min(block_space, size - data_idx);
        memcpy(write_cur_->data + block_pos, src + data_idx, write_size);
        data_idx += write_size;
        write_pos_ += write_size;
        if(write_pos_ % block_size_ == 0)
            write_cur_ = write_cur_->next;
    }
}

size_t ByteArray::read(void* buf, size_t size){
    size = std::min(size, get_unread_size());
    if(size == 0)
        return 0;
    size_t buf_idx = 0;
    uint8_t* dst = static_cast<uint8_t*>(buf);
    while(buf_idx < size){
        size_t block_pos = read_pos_ % block_size_;
        size_t block_space = block_size_ - block_pos;
        size_t read_size = std::min(block_space, size - buf_idx);
        memcpy(dst + buf_idx, read_cur_->data + block_pos, read_size);
        buf_idx += read_size;
        read_pos_ += read_size;
        if(read_pos_ % block_size_ == 0)
            read_cur_ = read_cur_->next;
    }
    return size;
}

void ByteArray::expand_capacity(size_t size){
    if(root_ == nullptr){
        root_ = new MemoryBlock(block_size_);
        write_cur_ = root_;
        read_cur_ = root_;
        capacity_ = block_size_;
    }
    MemoryBlock* cur_block = write_cur_;
    while(cur_block->next)
        cur_block = cur_block->next;
    while(get_spare_capacity() <= size){
        cur_block->next = new MemoryBlock(block_size_);
        cur_block = cur_block->next;
        capacity_ += block_size_;
    }
}

void ByteArray::write_fix_b(uint8_t value){
    write(&value, sizeof(value));
}
void ByteArray::write_fix_s(uint16_t value){
    value = Endian::host_to_network(value);
    write(&value, sizeof(value));
}
void ByteArray::write_fix_l(uint32_t value){
    value = Endian::host_to_network(value);
    write(&value, sizeof(value));
}
void ByteArray::write_fix_ll(uint64_t value){
    value = Endian::host_to_network(value);
    write(&value, sizeof(value));
}

template<typename T>
static T encode_zigzag(T v){
    return (v << 1) ^ (v >> (sizeof(T) * 8 - 1));
}
template<typename T>
static T decode_zigzag(T v){
    return (v >> 1) ^ -(v & 1);
}
void ByteArray::write_var_l(int32_t value){
    write_var_ul(encode_zigzag<int32_t>(value));
}
void ByteArray::write_var_ul(uint32_t value){
    uint8_t value_bytes[5], idx = 0;
    while(value >= 0x80){
        uint8_t byte = (value & 0x7F) | 0x80;
        value_bytes[idx++] = byte;
        value >>= 7;
    }
    value_bytes[idx++] = value;
    write(value_bytes, idx);
}
void ByteArray::write_var_ll(int64_t value){
    write_var_ull(encode_zigzag<int64_t>(value));
}
void ByteArray::write_var_ull(uint64_t value){
    uint8_t value_bytes[10], idx = 0;
    while(value >= 0x80){
        uint8_t byte = (value & 0x7F) | 0x80;
        value_bytes[idx++] = byte;
        value >>= 7;
    }
    value_bytes[idx++] = value;
    write(value_bytes, idx);
}

void ByteArray::write_float(float value){
    write(&value, sizeof(value));
}
void ByteArray::write_double(double value){
    write(&value, sizeof(value));
}

void ByteArray::write_string(const std::string& value){
    write(value.c_str(), value.size());
}
void ByteArray::write_string_with_length(const std::string& value){
    write_var_ul(value.size());
    write_string(value);
}


#define READ_FIX_IMPL(type, func_suffix) \
    type ByteArray::read_fix_##func_suffix(){ \
        type value; \
        size_t read_size = read(&value, sizeof(type)); \
        if(read_size != sizeof(type)){ \
            set_read_position(read_pos_ - read_size); \
            throw std::out_of_range("ByteArray::read_fix_" #func_suffix " out of range"); \
        } \
        return Endian::network_to_host(value); \
    }
READ_FIX_IMPL(int8_t, b)
READ_FIX_IMPL(uint8_t, ub)
READ_FIX_IMPL(int16_t, s)
READ_FIX_IMPL(uint16_t, us)
READ_FIX_IMPL(int32_t, l)
READ_FIX_IMPL(uint32_t, ul)
READ_FIX_IMPL(int64_t, ll)
READ_FIX_IMPL(uint64_t, ull)
#undef READ_FIX_IMPL

int32_t ByteArray::read_var_l(){
    return decode_zigzag<uint32_t>(read_var_ul());
}
uint32_t ByteArray::read_var_ul(){
    uint32_t result = 0;
    for(int i = 0; i < 5; ++i){
        uint8_t byte;
        size_t read_size = read(&byte, sizeof(byte));
        if(read_size != sizeof(byte)){
            set_read_position(read_pos_ - read_size - i * sizeof(byte));
            throw std::out_of_range("ByteArray::read_var_ul out of range");
        }
        if(byte < 0x80){
            result |= ((u_int32_t)byte << (7 * i));
            break;
        }
        else{
            result |= (((u_int32_t)byte & 0x7F) << (7 * i));
        }
    }
    return result;
}
int64_t ByteArray::read_var_ll(){
    return decode_zigzag<uint64_t>(read_var_ull());
}
uint64_t ByteArray::read_var_ull(){
    uint64_t result = 0;
    for(int i = 0; i < 10; ++i){
        uint8_t byte;
        size_t read_size = read(&byte, sizeof(byte));
        if(read_size != sizeof(byte)){
            set_read_position(read_pos_ - read_size - i * sizeof(byte));
            throw std::out_of_range("ByteArray::read_var_ull out of range");
        }
        if(byte < 0x80){
            result |= ((u_int64_t)byte << (7 * i));
            break;
        }
        else{
            result |= (((u_int64_t)byte & 0x7F) << (7 * i));
        }
    }
    return result;
}

float ByteArray::read_float(){
    float value;
    size_t read_size = read(&value, sizeof(value));
    if(read_size != sizeof(value)){
        set_read_position(read_pos_ - read_size);
        throw std::out_of_range("ByteArray::read_float out of range");
    }
    return value;
}
double ByteArray::read_double(){
    double value;
    size_t read_size = read(&value, sizeof(value));
    if(read_size != sizeof(value)){
        set_read_position(read_pos_ - read_size);
        throw std::out_of_range("ByteArray::read_double out of range");
    }
    return value;
}

std::string ByteArray::read_string(){
    uint32_t length = read_fix_ul();
    std::string str;
    str.resize(length);
    size_t read_size = read(&str[0], length);
    if(read_size != length){
        set_read_position(read_pos_ - read_size - sizeof(length));
        throw std::out_of_range("ByteArray::read_string out of range");
    }
    return str;
}
std::string ByteArray::read_string_with_length(size_t length){
    std::string str;
    str.resize(length);
    size_t read_size = read(&str[0], length);
    if(read_size != length){
        set_read_position(read_pos_ - read_size);
        throw std::out_of_range("ByteArray::read_string_with_length out of range");
    }
    return str;
}

void ByteArray::write_to_file(const std::string& filename){
    FILE* fp = fopen(filename.c_str(), "wb");
    if(!fp)
        throw std::logic_error("ByteArray::write_to_file open file failed");
    size_t old_read_pos = get_read_position();
    set_read_position(0);
    size_t to_write = write_pos_;
    while(to_write > 0){
        size_t block_pos = read_pos_ % block_size_;
        size_t block_space = block_size_ - block_pos;
        size_t write_size = std::min(block_space, to_write);
        size_t n = fwrite(read_cur_->data + block_pos, 1, write_size, fp);
        if(n != write_size){
            set_read_position(old_read_pos);
            fclose(fp);
            throw std::logic_error("ByteArray::write_to_file write file failed");
        }
        read_pos_ += write_size;
        to_write -= write_size;
        if(read_pos_ % block_size_ == 0)
            read_cur_ = read_cur_->next;
    }
    set_read_position(old_read_pos);
    fclose(fp);
}
void ByteArray::read_from_file(const std::string& filename){
    FILE* fp = fopen(filename.c_str(), "rb");
    if(!fp)
        throw std::logic_error("ByteArray::read_from_file open file failed");
    clear();
    root_ = new MemoryBlock(block_size_);
    read_cur_ = root_;
    write_cur_ = root_;
    capacity_ = block_size_;
    size_t n = 0;
    do{
        expand_capacity(block_size_);
        n = fread(write_cur_->data, 1, block_size_, fp);
        write_pos_ += n;
        if(n == block_size_)
            write_cur_ = write_cur_->next;
    }while(n == block_size_);
    read_pos_ = 0;
    read_cur_ = root_;
    fclose(fp);
}

void ByteArray::set_read_position(size_t pos){
    if(pos > write_pos_)
        throw std::out_of_range("ByteArray::set_read_position out of range");
    read_pos_ = pos;
    read_cur_ = root_;
    size_t block_idx = pos / block_size_;
    while(block_idx--)
        read_cur_ = read_cur_->next;
}

void ByteArray::get_read_buffers(std::vector<iovec>& buffers, size_t length) const{
    length = std::min(length, get_unread_size());
    if(length == 0)
        return;
    size_t tmp_read_pos = read_pos_;
    MemoryBlock* cur_block = read_cur_;
    while(length > 0){
        size_t block_pos = tmp_read_pos % block_size_;
        size_t block_space = block_size_ - block_pos;
        size_t read_size = std::min(block_space, length);
        iovec iov;
        iov.iov_base = cur_block->data + block_pos;
        iov.iov_len = read_size;
        buffers.push_back(iov);

        tmp_read_pos += read_size;
        length -= read_size;
        if(tmp_read_pos % block_size_ == 0)
            cur_block = cur_block->next;
    }
}
void ByteArray::get_write_buffers(std::vector<iovec>& buffers, size_t length){
    if(length == 0)
        return;
    expand_capacity(length);
    size_t tmp_write_pos = write_pos_;
    MemoryBlock* cur_block = write_cur_;
    while(length > 0){
        size_t block_pos = tmp_write_pos % block_size_;
        size_t block_space = block_size_ - block_pos;
        size_t write_size = std::min(block_space, length);
        iovec iov;
        iov.iov_base = cur_block->data + block_pos;
        iov.iov_len = write_size;
        buffers.push_back(iov);

        tmp_write_pos += write_size;
        length -= write_size;
        if(tmp_write_pos % block_size_ == 0)
            cur_block = cur_block->next;
    }
}

std::string ByteArray::to_hex_string(){
    std::stringstream ss;
    std::string str;
    size_t old_read_pos = get_read_position();
    str.resize(get_unread_size());
    read(&str[0], str.size());
    for(size_t i = 0; i < str.size(); ++i){
        uint8_t byte = str[i];
        ss << std::hex;
        ss.width(2);
        ss.fill('0');
        ss << (int)(byte);
    }
    set_read_position(old_read_pos);
    return ss.str();
}

void ByteArray::clear(){
    MemoryBlock* cur = root_;
    while(cur){
        MemoryBlock* next = cur->next;
        delete cur;
        cur = next;
    }
    root_ = read_cur_ = write_cur_ = nullptr;
    write_pos_ = read_pos_ = capacity_ = 0;
}

}