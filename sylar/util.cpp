#include "util.h"
#include "fiber.h"
#include <thread>
#include <arpa/inet.h>

namespace sylar{

pid_t get_thread_id(){
    return syscall(SYS_gettid);
}

fid_t get_fiber_id(){
    return sylar::Fiber::get_this()->get_id();
}


void backtrace(std::vector<std::string>& buffer, int size, int skip){
    buffer.clear();
    void** buf = (void**)malloc(sizeof(void*) * size);
    int nptrs = ::backtrace(buf, size);
    char** strings = ::backtrace_symbols(buf, nptrs);
    free(buf);
    if(strings != nullptr){
        for(int i = skip; i < nptrs; ++i)
            buffer.push_back(strings[i]);
        free(strings);
    }
}
std::string backtrace_symbols(int size, int skip, const std::string& prefix){
    std::vector<std::string> bt;
    backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0; i < bt.size(); ++i){
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

namespace Endian{
    uint8_t host_to_network(uint8_t value){
        return value;
    }
    uint16_t host_to_network(uint16_t value){
        return htons(value);
    }
    uint32_t host_to_network(uint32_t value){
        return htonl(value);
    }
    uint64_t host_to_network(uint64_t value){
        return (((uint64_t)htonl(value & 0xFFFFFFFFULL)) << 32) | htonl(value >> 32);
    }

    uint8_t network_to_host(uint8_t value){
        return value;
    }
    uint16_t network_to_host(uint16_t value){
        return ntohs(value);
    }
    uint32_t network_to_host(uint32_t value){
        return ntohl(value);
    }
    uint64_t network_to_host(uint64_t value){
        return (((uint64_t)ntohl(value & 0xFFFFFFFFULL)) << 32) | ntohl(value >> 32);
    }
    int8_t network_to_host(int8_t value){
        return value;
    }
    int16_t network_to_host(int16_t value){
        return network_to_host((uint16_t)value);
    }
    int32_t network_to_host(int32_t value){
        return network_to_host((uint32_t)value);
    }
    int64_t network_to_host(int64_t value){
        return network_to_host((uint64_t)value);
    }
}

namespace TimeUtil{
    uint64_t get_time_ms(){
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
    }
}

namespace FSUtil{
    bool open_for_read(::std::ifstream& ifs, const ::std::string& file_name, ::std::ios_base::openmode mode){
        ifs.open(file_name, mode);
        return (bool)ifs;
    }
    bool open_for_write(::std::ofstream& ofs, const ::std::string& file_name, ::std::ios_base::openmode mode){
        ofs.open(file_name, mode);
        if((bool)ofs)
            return true;
        
        std::string dir = get_directory(file_name);
        make_dir(dir);
        ofs.open(file_name, mode);
        return (bool)ofs;
    }

    bool file_or_dir_exists(const std::string& path){
        return access(path.c_str(), F_OK) == 0;
    }

    bool make_dir(const std::string& path){
        if(access(path.c_str(), F_OK) == 0)
            return true;
        std::string tmp = path;
        size_t pos = tmp.find('/', 1);
        while(pos != tmp.npos){
            tmp[pos] = '\0';
            if(file_or_dir_exists(tmp) == false){
                if(mkdir(tmp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                    return false;
            }
            tmp[pos] = '/';
            pos = tmp.find('/', pos + 1);
        }
        if(mkdir(tmp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
            return false;
        return true;
    }

    std::string get_directory(const std::string& path){
        if(path.empty())
            return ".";
        size_t pos = path.find_last_of('/');
        if(pos == path.npos)
            return ".";
        else if(pos == 0)
            return "/";
        else
            return path.substr(0, pos);
    }

}




}