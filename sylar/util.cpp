#include "util.h"
#include <thread>

namespace sylar{

pid_t get_thread_id(){
    return syscall(SYS_gettid);
}

fid_t get_fiber_id(){
    return 666;
}



namespace FSUtil{
    bool open_for_read(::std::ifstream& ifs, const ::std::string& file_name, ::std::ios_base::openmode mode){
        ifs.open(file_name, mode);
        return (bool)ifs;
    }
    bool open_for_write(::std::ofstream& ofs, const ::std::string& file_name, ::std::ios_base::openmode mode){
        if(file_or_dir_exists(file_name) == false){
            std::string dir = get_directory(file_name);
            if(make_dir(dir) == false)
                return false;
        }
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
        size_t pos = tmp.find('/', 0);
        while(pos != tmp.npos){
            tmp[pos] = '\0';
            if(mkdir(tmp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                return false;
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