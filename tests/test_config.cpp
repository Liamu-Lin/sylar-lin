
#include "log.h"
#include "config.h"
#include <iostream>

#include <yaml-cpp/yaml.h>

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::ConfigMgr.look_up<int>("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::ConfigMgr.look_up<float>("system.value", (float)10.2f, "system value");

sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_config =
    sylar::ConfigMgr.look_up<std::vector<int>>("int_vec", std::vector<int>{3,2,1}, "int vector");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << "scalar";
    } else if(node.IsNull()) {
        SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << std::string(level * 4, ' ')
            << "NULL - " << "null";
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << std::string(level * 4, ' ')
                    << it->first << " - " << "map";
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << std::string(level * 4, ' ')
                << i << " - " << "sequence";
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/log.yml");
    print_yaml(root, 0);
}

void test_config(){
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "Before: "
        << "g_int_value_config = " << g_int_value_config->get_value()
        << " g_float_value_config = " << g_float_value_config->get_value()
        << " g_int_vec_config = " << g_int_vec_config->to_string();

        YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/log.yml");
    sylar::ConfigMgr.load_from_yaml(root);

    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "After: "
        << "g_int_value_config = " << g_int_value_config->get_value()
        << " g_float_value_config = " << g_float_value_config->get_value()
        << " g_int_vec_config = " << g_int_vec_config->to_string();

    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_int_vec_config get_value:";
    for(auto& i : g_int_vec_config->get_value()){
        SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << i;
    }

}

int main(int argc, char** argv){
    // auto root = sylar::LoggerMgr.get_root();
    
    auto lg = sylar::Logger::ptr(new sylar::Logger("system"));
    lg->set_level(sylar::LogLevel::Level::INFO);
    auto ap = sylar::LogAppender::ptr(new sylar::StdoutLogAppender);
    ap->set_level(sylar::LogLevel::Level::INFO);
    lg->add_appender(ap);

    auto lg2 = sylar::LoggerMgr.get_logger("system2");

    SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
        << "g_int_value_config = " << g_int_value_config->get_value()
        << " g_float_value_config = " << g_float_value_config->get_value();

    SYLAR_LOG(lg2, sylar::LogLevel::Level::INFO)
        << "g_int_value_config to_string = " << g_int_value_config->to_string()
        << " g_float_value_config to_string = " << g_float_value_config->to_string();

    g_int_value_config->from_string("1024");
    g_float_value_config->from_string("3.14159");

    SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
        << "g_int_value_config = " << g_int_value_config->get_value()
        << " g_float_value_config = " << g_float_value_config->get_value();

    //test_yaml();

    test_config();

    return 0;
}