#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <execinfo.h>

#include <stdint.h>
#include <unistd.h>

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace sylar{

typedef uint32_t fid_t;

pid_t get_thread_id();
//TODO
fid_t get_fiber_id();


void backtrace(std::vector<std::string>& buffer, int size, int skip = 0);
std::string backtrace_symbols(int size, int skip = 0, const std::string& prefix = "");


namespace FSUtil{
    bool open_for_read(::std::ifstream& ifs, const ::std::string& file_name, 
        ::std::ios_base::openmode mode = ::std::ios_base::in);
    bool open_for_write(::std::ofstream& ofs, const ::std::string& file_name, 
        ::std::ios_base::openmode mode = ::std::ios_base::out);
    bool file_or_dir_exists(const std::string& path);
    bool make_dir(const std::string& path);
    std::string get_directory(const std::string& path);
}


}





#endif