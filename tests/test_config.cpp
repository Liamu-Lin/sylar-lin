
#include "log.h"
#include "config.h"
#include <iostream>

#include <yaml-cpp/yaml.h>

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::ConfigMgr.look_up("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::ConfigMgr.look_up("system.value", (float)10.2f, "system value");

sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_config =
    sylar::ConfigMgr.look_up("system.int_vec", std::vector<int>{3,2,1}, "system int_vec");

sylar::ConfigVar<std::list<int>>::ptr g_int_lst_config =
    sylar::ConfigMgr.look_up("system.int_lst", std::list<int>{3,2,1}, "system int_lst");

sylar::ConfigVar<std::set<int>>::ptr g_int_set_config =
    sylar::ConfigMgr.look_up("system.int_set", std::set<int>{1,2,3}, "system int_set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_config =
    sylar::ConfigMgr.look_up("system.int_uset", std::unordered_set<int>{1,2,3}, "system int_uset");

sylar::ConfigVar<std::map<std::string, int>>::ptr g_int_map_config =
    sylar::ConfigMgr.look_up("system.int_map", std::map<std::string, int>{{"k1", 1}, {"k2", 2}, {"k3", 3}}, "system int_map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_int_umap_config =
    sylar::ConfigMgr.look_up("system.int_umap", std::unordered_map<std::string, int>{{"u1", 1}, {"u2", 2}, {"u3", 3}}, "system int_umap");

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
    #define LOG_VAR_IN_STR(var, prefix) \
        SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
        << " " << #var << " = " << var->to_string();
    #define LOG_VAR_IN_VALUE(var, prefix) \
        {   \
            auto v = var->get_value(); \
            for(auto& i : v){   \
                SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
                << " " << #var << " value = " << i; \
            }   \
        }
    #define LOG_MAP_IN_STR(var, prefix) \
        {\
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
            << " " << #var << " = " << var->to_string(); \
        }
    #define LOG_MAP_IN_VALUE(var, prefix) \
        {   \
            auto v = var->get_value(); \
            for(auto& i : v){   \
                SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
                << " " << #var << " value = {" << i.first << " : " << i.second << "}"; \
            }   \
        }

    LOG_VAR_IN_STR(g_int_value_config, "Before");
    LOG_VAR_IN_STR(g_float_value_config, "Before");
    LOG_VAR_IN_STR(g_int_vec_config, "Before");
    LOG_VAR_IN_STR(g_int_lst_config, "Before");
    LOG_VAR_IN_STR(g_int_set_config, "Before");
    LOG_VAR_IN_STR(g_int_uset_config, "Before");
    LOG_MAP_IN_STR(g_int_map_config, "Before");
    LOG_MAP_IN_STR(g_int_umap_config, "Before");

    LOG_VAR_IN_VALUE(g_int_vec_config, "Before");
    LOG_VAR_IN_VALUE(g_int_lst_config, "Before");
    LOG_VAR_IN_VALUE(g_int_set_config, "Before");
    LOG_VAR_IN_VALUE(g_int_uset_config, "Before");
    LOG_MAP_IN_VALUE(g_int_map_config, "Before");
    LOG_MAP_IN_VALUE(g_int_umap_config, "Before");

    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/log.yml");
    sylar::ConfigMgr.load_from_yaml(root);

    LOG_VAR_IN_STR(g_int_value_config, "After");
    LOG_VAR_IN_STR(g_float_value_config, "After");
    LOG_VAR_IN_STR(g_int_vec_config, "After");
    LOG_VAR_IN_STR(g_int_lst_config, "After");
    LOG_VAR_IN_STR(g_int_set_config, "After");
    LOG_VAR_IN_STR(g_int_uset_config, "After");
    LOG_MAP_IN_STR(g_int_map_config, "After");
    LOG_MAP_IN_STR(g_int_umap_config, "After");

    LOG_VAR_IN_VALUE(g_int_vec_config, "After");
    LOG_VAR_IN_VALUE(g_int_lst_config, "After");
    LOG_VAR_IN_VALUE(g_int_set_config, "After");
    LOG_VAR_IN_VALUE(g_int_uset_config, "After");
    LOG_MAP_IN_VALUE(g_int_map_config, "After");
    LOG_MAP_IN_VALUE(g_int_umap_config, "After");

}

int main(int argc, char** argv){
    // auto root = sylar::LoggerMgr.get_root();
    
    // auto lg = sylar::Logger::ptr(new sylar::Logger("system"));
    // lg->set_level(sylar::LogLevel::Level::INFO);
    // auto ap = sylar::LogAppender::ptr(new sylar::StdoutLogAppender);
    // ap->set_level(sylar::LogLevel::Level::INFO);
    // lg->add_appender(ap);

    // auto lg2 = sylar::LoggerMgr.get_logger("system2");

    // SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
    //     << "g_int_value_config = " << g_int_value_config->get_value()
    //     << " g_float_value_config = " << g_float_value_config->get_value();

    // SYLAR_LOG(lg2, sylar::LogLevel::Level::INFO)
    //     << "g_int_value_config to_string = " << g_int_value_config->to_string()
    //     << " g_float_value_config to_string = " << g_float_value_config->to_string();

    // g_int_value_config->from_string("1024");
    // g_float_value_config->from_string("3.14159");

    // SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
    //     << "g_int_value_config = " << g_int_value_config->get_value()
    //     << " g_float_value_config = " << g_float_value_config->get_value();

    //test_yaml();

    test_config();

    return 0;
}