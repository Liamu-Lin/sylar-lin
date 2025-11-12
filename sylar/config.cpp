#include "config.h"

#include <list>
#include <sstream>

namespace sylar{

ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& description):
    name_(name),
    description_(description){
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
}
ConfigVarBase::~ConfigVarBase() {
    ;
}

Config::Config(){
    ;
}

bool load_all_member(const YAML::Node& node, 
                     const std::string& prefix,
                     std::list<std::pair<std::string, const YAML::Node>>& output){
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") != std::string::npos) {
        SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::ERROR) << "Config load_all_member invalid prefix " << prefix;
        return false;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()){
        for(auto it = node.begin(); it != node.end(); ++it){
            size_t prefix_len = prefix.length();
            std::string cur = (prefix_len == 0) ? it->first.Scalar() : prefix + '.' + it->first.Scalar();
            std::transform(cur.begin() + prefix_len, cur.end(), cur.begin() + prefix_len, ::tolower);
            load_all_member(it->second, cur, output);
        }
    }
    return true;
}
bool Config::load_from_yaml(const YAML::Node& root){
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    load_all_member(root, "", all_nodes);
    for(auto& it : all_nodes){
        std::string& prefix = it.first;
        const YAML::Node& node = it.second;
        auto var = look_up_base(prefix);
        if(var){
            if(node.IsScalar()){
                var->from_string(node.Scalar());
            }
            else{
                std::stringstream ss;
                ss << node;
                var->from_string(ss.str());
            }
        }
    }


    return true;
}


}