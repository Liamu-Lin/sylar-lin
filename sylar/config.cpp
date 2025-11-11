#include "config.h"


namespace sylar{

ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& description):
    name_(name),
    description_(description){
    ;
}
ConfigVarBase::~ConfigVarBase() {
    ;
}

Config::Config(){
    ;
}



}