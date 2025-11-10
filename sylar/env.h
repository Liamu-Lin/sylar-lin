#ifndef __SYLAR_ENV__
#define __SYLAR_ENV__

#include "singleton.h"

#include <map>
#include <vector>
#include <string>

namespace sylar{

class Env: public Singleton<Env>{
    Singleton_Constructor(Env);
public:
    #define EnvMgr Singleton<Env>::Instance()

    bool init(int argc, char** argv);

    void set(const std::string& key, const std::string& val);
    bool del(const std::string& key);
    bool has(const std::string& key);
    std::string get(const std::string& key, const std::string& default_value = "");

    void set_help(const std::string& key, const std::string& desc);
    void remove_help(const std::string& key);
    void print_help();

    bool set_env(const std::string& key, const std::string& value);
    std::string get_env(const std::string& key, const std::string& default_value = "") const;

    const std::string& getExe() const { return exe_;}
    const std::string& getCwd() const { return cwd_;}
private:
    std::map<std::string, std::string> args_;
    std::map<std::string, std::string> helps_;
    
    std::string exe_;       // /proc/<pid>/exe
    std::string cwd_;
    std::string program_name_;
};



}

#endif