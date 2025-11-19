#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include "log.h"
#include "mutex.h"
#include "singleton.h"

#include <sstream>
#include <string>
#include <exception>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include <functional>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>


namespace sylar{

template<typename F, typename T>
class Converter{
public:
    static T convert(const F& v){
        return boost::lexical_cast<T>(v);
    }
};
// partial specialization for YAML str <-> vector<T>
template<typename T>
class Converter<std::string, std::vector<T>>{
public:
    static std::vector<T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.push_back(Converter<std::string, T>::convert(ss.str()));
        }
        return vec;
    }
};
template<typename T>
class Converter<std::vector<T>, std::string>{
public:
    static std::string convert(const std::vector<T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Sequence);
        for(const auto& i : v)
            node.push_back(YAML::Load(Converter<T, std::string>::convert(i)));
        ss << node;
        return ss.str();
    }
};
// partial specialization for YAML str <-> list<T>
template<typename T>
class Converter<std::string, std::list<T>>{
public:
    static std::list<T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::list<T> lst;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            lst.push_back(Converter<std::string, T>::convert(ss.str()));
        }
        return lst;
    }
};
template<typename T>
class Converter<std::list<T>, std::string>{
public:
    static std::string convert(const std::list<T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Sequence);
        for(const auto& i : v)
            node.push_back(YAML::Load(Converter<T, std::string>::convert(i)));
        ss << node;
        return ss.str();
    }
};
// partial specialization for YAML str <-> set<T>
template<typename T>
class Converter<std::string, std::set<T>>{
public:
    static std::set<T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::set<T> st;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            st.insert(Converter<std::string, T>::convert(ss.str()));
        }
        return st;
    }
};
template<typename T>
class Converter<std::set<T>, std::string>{
public:
    static std::string convert(const std::set<T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Sequence);
        for(const auto& i : v)
            node.push_back(YAML::Load(Converter<T, std::string>::convert(i)));
        ss << node;
        return ss.str();
    }
};
// partial specialization for YAML str <-> unordered_set<T>
template<typename T>
class Converter<std::string, std::unordered_set<T>>{
public:
    static std::unordered_set<T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::unordered_set<T> ust;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            ust.insert(Converter<std::string, T>::convert(ss.str()));
        }
        return ust;
    }
};
template<typename T>
class Converter<std::unordered_set<T>, std::string>{
public:
    static std::string convert(const std::unordered_set<T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Sequence);
        for(const auto& i : v)
            node.push_back(YAML::Load(Converter<T, std::string>::convert(i)));
        ss << node;
        return ss.str();
    }
};
// partial specialization for YAML str <-> map<std::string, T>
template<typename T>
class Converter<std::string, std::map<std::string, T>>{
public:
    static std::map<std::string, T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::map<std::string, T> mp;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it){
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(it->first.Scalar(), 
                      Converter<std::string, T>::convert(ss.str())));
        }
        return mp;
    }
};
template<typename T>
class Converter<std::map<std::string, T>, std::string>{
public:
    static std::string convert(const std::map<std::string, T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Map);
        for(const auto& i : v)
            node[i.first] = YAML::Load(Converter<T, std::string>::convert(i.second));
        ss << node;
        return ss.str();
    }
};
// partial specialization for YAML str <-> unordered_map<std::string, T>
template<typename T>
class Converter<std::string, std::unordered_map<std::string, T>>{
public:
    static std::unordered_map<std::string, T> convert(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::unordered_map<std::string, T> mp;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it){
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(it->first.Scalar(), 
                      Converter<std::string, T>::convert(ss.str())));
        }
        return mp;
    }
};
template<typename T>
class Converter<std::unordered_map<std::string, T>, std::string>{
public:
    static std::string convert(const std::unordered_map<std::string, T>& v){
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Map);
        for(const auto& i : v)
            node[i.first] = YAML::Load(Converter<T, std::string>::convert(i.second));
        ss << node;
        return ss.str();
    }
};


class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    typedef std::shared_ptr<const ConfigVarBase> const_ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "");
    virtual ~ConfigVarBase() = 0;

    const std::string& get_name() const { return name_; }
    const std::string& get_description() const {return description_; }
    virtual std::string get_type_name() const = 0;

    //convert to/from string
    virtual std::string to_string() const = 0;
    virtual bool from_string(const std::string& str) = 0;

private:
    // lower-case name
    std::string name_;
    std::string description_;
};

template<typename T, typename FromStr = Converter<std::string, T>
                   , typename ToStr = Converter<T, std::string>>
class ConfigVar : public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const std::string& description, const T& value):
        ConfigVarBase(name, description),
        value_(value){
        ;
    }

    ~ConfigVar() override{}

    std::string get_type_name() const override{
        return typeid(T).name();
    }

    std::string to_string() const override{
        try{
            RWMutex::RLock lock(value_mutex_);
            return ToStr::convert(value_);
        } catch(std::exception& e){
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "ConfigVar::to_string exception: "
                << e.what() << " name=" << get_name() << " type=" << get_type_name();
        }
        return "";
    }
    bool from_string(const std::string& str) override{
        try{
            set_value(FromStr::convert(str));
            return true;
        } catch(std::exception& e){
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "ConfigVar::from_string exception: "
                << e.what() << " name=" << get_name() << " type=" << get_type_name() << " str=" << str;
        }
        return false;
    }

    T get_value() const { return value_; }
    void set_value(const T& value) {
        {
            RWMutex::RLock value_lock(value_mutex_);
            if(value == value_)
                return;
            RWMutex::RLock cbs_lock(cbs_mutex_);
            for(auto& f : cbs_)
                f.second(value_, value);
        }
        RWMutex::WLock value_lock(value_mutex_);
        value_ = value;
    }

    uint64_t add_listener(on_change_cb cb){
        static uint64_t s_fun_id = 0;
        RWMutex::WLock lock(cbs_mutex_);
        cbs_[++s_fun_id] = cb;
        return s_fun_id;
    }
    void del_listener(uint64_t key){
        RWMutex::WLock lock(cbs_mutex_);
        cbs_.erase(key);
    }
    void clear_listeners(){
        RWMutex::WLock lock(cbs_mutex_);
        cbs_.clear();
    }
    on_change_cb get_listener(uint64_t key){
        RWMutex::RLock lock(cbs_mutex_);
        auto it = cbs_.find(key);
        return it == cbs_.end() ? nullptr : it->second;
    }

private:
    T value_;
    std::map<uint64_t, on_change_cb> cbs_;
    mutable RWMutex value_mutex_;
    RWMutex cbs_mutex_;
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
        RWMutex::WLock lock(datas_mutex_);
        //search or create
        auto it = datas_.find(name);
        std::shared_ptr<ConfigVar<T>> ptr = nullptr;
        if(it != datas_.end()){
            ptr = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(ptr){
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name = " << name << " exists";
                return ptr;
            }
            else{
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "look_up name = " << name << " exists but type not match, "
                    << "real_type=" << it->second->get_type_name() << " req_type=" << typeid(T).name();
                return nullptr;
            }
        }
        else{
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name = " << name << " not exists, create it";
            ptr.reset(new ConfigVar<T>(name, description, default_value));
            datas_[name] = ptr;
        }
        return ptr;
    }
    // search only
    template<typename T>
    std::shared_ptr<ConfigVar<T>> look_up(const std::string& name){
        // check name validity
        if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") != std::string::npos) {
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        RWMutex::RLock lock(datas_mutex_);
        //search
        auto it = datas_.find(name);
        std::shared_ptr<ConfigVar<T>> ptr = nullptr;
        // exists
        if(it != datas_.end()){
            ptr = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(ptr){
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name = " << name << " exists";
            }
            else{
                SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "look_up name = " << name << " exists but type not match, "
                    << "real_type=" << it->second->get_type_name() << " req_type=" << typeid(T).name();
            }
        }
        //not exists
        else
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "look_up name = " << name << " not exists";
        return ptr;
    }
    // search only and return base class pointer
    std::shared_ptr<ConfigVarBase> look_up_base(const std::string& name){
        RWMutex::RLock lock(datas_mutex_);
        auto it = datas_.find(name);
        return it == datas_.end() ? nullptr : it->second;
    }

    //load variables from YAML node
    bool load_from_yaml(const YAML::Node& root);
    void visit(std::function<void(ConfigVarBase::const_ptr)> cb);
private:
    // var name to var
    std::map<std::string, std::shared_ptr<ConfigVarBase>> datas_;
    RWMutex datas_mutex_;
};



}




#endif