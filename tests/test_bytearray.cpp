#include "log.h"
#include "util.h"
#include "macro.h"
#include "bytearry.h"
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <iomanip>

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

void test(void*){
    #define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    for(size_t i = 0; i < vec.size(); ++i) { \
        type v = ba->read_fun(); \
        if(v != vec[i]) { \
            std::cout << "i=" << i \
                      << " v=" << std::hex << v \
                      << " vec[i]=" << std::hex << vec[i] << std::endl; \
        } \
        SYLAR_ASSERT(v == vec[i]); \
    } \
    SYLAR_ASSERT(ba->get_unread_size() == 0); \
    SYLAR_LOG(g_logger, g_logger->get_level()) << #write_fun "/" #read_fun \
                    " (" #type " ) len=" << len \
                    << " base_len=" << base_len \
                    << " size=" << ba->get_size(); \
}

    XX(int8_t,   1000, write_fix_b, read_fix_b, 1);
    XX(int16_t,  1000, write_fix_s,  read_fix_s, 1);
    XX(int32_t,  1000, write_fix_l,  read_fix_l, 1);
    XX(int64_t,  1000, write_fix_ll,  read_fix_ll, 1);
    
    XX(int32_t,  1000, write_var_l,  read_var_l, 1);
    XX(uint32_t, 1000, write_var_ul, read_var_ul, 1);
    XX(int64_t,  1000, write_var_ll,  read_var_ll, 1);
    XX(uint64_t, 1000, write_var_ull, read_var_ull, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for(int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    for(size_t i = 0; i < vec.size(); ++i) { \
        type v = ba->read_fun(); \
        SYLAR_ASSERT(v == vec[i]); \
    } \
    SYLAR_ASSERT(ba->get_unread_size() == 0); \
    SYLAR_LOG(g_logger, g_logger->get_level()) << #write_fun "/" #read_fun \
                    " (" #type " ) len=" << len \
                    << " base_len=" << base_len \
                    << " size=" << ba->get_size(); \
    ba->write_to_file("/tmp/" #type "_" #len "-" #read_fun ".dat"); \
    ba->set_read_position(0); \
    sylar::ByteArray::ptr ba2(new sylar::ByteArray(base_len * 2)); \
    ba2->read_from_file("/tmp/" #type "_" #len "-" #read_fun ".dat"); \
    if(ba->to_hex_string() != ba2->to_hex_string()){ \
        std::cout << "ba:  " << ba->to_hex_string() << std::endl; \
        std::cout << "ba2: " << ba2->to_hex_string() << std::endl; \
    } \
    SYLAR_ASSERT(ba->get_read_position() == 0); \
    SYLAR_ASSERT(ba2->get_read_position() == 0); \
}
    XX(int8_t,  1000, write_fix_b, read_fix_b, 1);
    XX(int16_t,  1000, write_fix_s,  read_fix_s, 1);
    XX(int32_t,  1000, write_fix_l,  read_fix_l, 1);
    XX(int64_t,  1000, write_fix_ll,  read_fix_ll, 1);
    XX(int32_t,  1000, write_var_l,  read_var_l, 1);
    XX(uint32_t, 1000, write_var_ul, read_var_ul, 1);
    XX(int64_t,  1000, write_var_ll,  read_var_ll, 1);
    XX(uint64_t, 1000, write_var_ull, read_var_ull, 1);
#undef XX
}


int main(){
    srand(time(0));
    test(nullptr);

    return 0;
}