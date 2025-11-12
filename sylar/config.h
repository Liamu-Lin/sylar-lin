#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include "log.h"
#include "singleton.h"

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <map>

namespace sylar{

class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    ConfigVarBase(const std::string& name, const std::string& description = "");
    virtual ~ConfigVarBase() = 0;

    const std::string& get_name() const { return name_; }
    const std::string& get_description() const {return description_; }
    virtual std::string get_type_name() const = 0;

    //convert to/from string
    virtual std::string to_string() = 0;
    virtual bool from_string(const std::string& str) = 0;

private:
    // lower-case name
    std::string name_;
    std::string description_;
};

template<typename T>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name, const std::string& description, const T& value):
        ConfigVarBase(name, description),
        value_(value){
        ;
    }

    ~ConfigVar() override{}

    std::string get_type_name() const override{
        return typeid(T).name();
    }

    std::string to_string() override{
        try{
            return boost::lexical_cast<std::string>(value_);
        } catch(...){
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "ConfigVar::to_string exception"
                << " name=" << get_name() << " type=" << get_type_name();
        }
        return "";
    }
    bool from_string(const std::string& str) override{
        try{
            value_ = boost::lexical_cast<T>(str);
            return true;
        } catch(...){
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "ConfigVar::from_string exception"
                << " name=" << get_name() << " type=" << get_type_name() << " str=" << str;
        }
        return false;
    }

    T get_value() const { return value_; }
    void set_value(const T& value) { value_ = value; }

private:
    T value_;
};

class Config{
    Singleton_Constructor(Config)
public:
    #define ConfigMgr Singleton<sylar::Config>::Instance()
    typedef std::shared_ptr<Config> ptr;

    // search and create if not exists
    template<typename T>
    std::shared_ptr<ConfigVar<T>> look_up(const std::string& name, const T& default_value, 
        const std::string& description = ""){
        // check name validity
        if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") != std::string::npos) {
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        //search or create
        auto ptr = look_up<T>(name);
        if(ptr == nullptr){
            ptr.reset(new ConfigVar<T>(name, description, default_value));
            datas_[name] = ptr;
        }
        return ptr;
    }
    // search only
    template<typename T>
    std::shared_ptr<ConfigVar<T>> look_up(const std::string& name){
        auto it = datas_.find(name);
        std::shared_ptr<ConfigVar<T>> ptr = nullptr;
        // exists
        if(it != datas_.end()){
            ptr = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(ptr){
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name=" << name << " exists";
            }
            else{
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "look_up name=" << name << " exists but type not match, "
                    << "real_type=" << it->second->get_type_name() << " req_type=" << typeid(T).name();
            }
        }
        //not exists
        else
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name=" << name << " not exists";
        return ptr;
    }
    // search only and return base class pointer
    std::shared_ptr<ConfigVarBase> look_up_base(const std::string& name){
        auto it = datas_.find(name);
        return it == datas_.end() ? nullptr : it->second;
    }

    //load variables from YAML node
    bool load_from_yaml(const YAML::Node& root);

private:
    // var name to var
    std::map<std::string, std::shared_ptr<ConfigVarBase>> datas_;
};



}




#endif