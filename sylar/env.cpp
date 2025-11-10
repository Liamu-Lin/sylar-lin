#include "env.h"
#include "log.h"

#include <unistd.h>
#include <string.h>
#include <iomanip>

namespace sylar{

static Logger::ptr g_logger = LoggerMgr.get_logger("system");

bool Env::init(int argc, char** argv){
    // init current working dirtory
    char link[256], path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    readlink(link, path, sizeof(path));
    exe_ = path;
    cwd_ = exe_.substr(0, exe_.find_last_of('/')) + '/';
    
    // init enveriment variable
    program_name_ = argv[0];
    const char* now_key = nullptr;
    for(int i = 1; i < argc; ++i){
        if(argv[i][0] == '-'){
            if(strlen(argv[i]) <= 1){
                SYLAR_LOG(g_logger, LogLevel::Level::ERROR) << "invalid arg idx = " << i
                                                            << ", val = " << argv[i];
                return false;
            }
            if(now_key != nullptr)
                args_[now_key] = "";
            now_key = argv[i] + 1;
        }
        else{
            if(now_key != nullptr){
                args_[now_key] = argv[i];
                now_key = nullptr;
            }
            else{
                SYLAR_LOG(g_logger, LogLevel::Level::ERROR) << "invalid arg idx = " << i
                                                            << ", val = " << argv[i];
                return false;
            }
        }
    }
    return true;
}

void Env::set(const std::string& key, const std::string& val){
    args_[key] = val;
}
bool Env::del(const std::string& key){
    return args_.erase(key) != 0;
}
bool Env::has(const std::string& key){
    return args_.count(key) != 0;
}
std::string Env::get(const std::string& key, const std::string& default_value){
    auto it = args_.find(key);
    if(it != args_.end())
        return it->second;
    return default_value;
}

void Env::set_help(const std::string& key, const std::string& desc){
    helps_[key] = desc;
}
void Env::remove_help(const std::string& key){
    helps_.erase(key);
}
void Env::print_help(){
    std::cout << program_name_ << " Help Manual:" << std::endl;
    std::cout << "[options] ------ usages" << std::endl;
    for(auto& [key, desc]: helps_)
        std::cout << std::setw(5) << '-' << key << " : " << desc << std::endl;
}

bool Env::set_env(const std::string& key, const std::string& value){
    return setenv(key.c_str(), value.c_str(), 1) == 0;
}
std::string Env::get_env(const std::string& key, const std::string& default_value) const{
    char* val = getenv(key.c_str());
    if(val == nullptr)
        return default_value;
    return val;
}


}