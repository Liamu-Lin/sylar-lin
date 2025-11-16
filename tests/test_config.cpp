
#include "log.h"
#include "config.h"
#include <iostream>

#include <yaml-cpp/yaml.h>

#if 0
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
    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/test.yml");
    print_yaml(root, 0);
}

void test_config(){
    sylar::LoggerMgr.add_logger("test");
    #define LOG_VAR_IN_STR(var, prefix) {\
        if(var == nullptr) {\
            SYLAR_LOG(sylar::LoggerMgr.get_logger("test"), sylar::LogLevel::Level::DEBUG) << "var " << #var << " is null"; \
        } \
        else {\
            SYLAR_LOG(sylar::LoggerMgr.get_logger("test"), sylar::LogLevel::Level::DEBUG) << #prefix \
            << " " << #var << " = " << var->to_string(); \
        } \
    }
    #define LOG_VAR_IN_VALUE(var, prefix) {\
        if(var == nullptr) {\
            SYLAR_LOG(sylar::LoggerMgr.get_logger("test"), sylar::LogLevel::Level::DEBUG) << "var " << #var << " is null"; \
        } \
        else {\
            auto v = var->get_value(); \
            for(auto& i : v){   \
                SYLAR_LOG(sylar::LoggerMgr.get_logger("test"), sylar::LogLevel::Level::DEBUG) << #prefix \
                << " " << #var << " value = " << i; \
            } \
        } \
    }
    #define LOG_MAP_IN_STR(var, prefix) {\
        if(var == nullptr) {\
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "var " << #var << " is null"; \
        } \
        else{ \
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
                << " " << #var << " = " << var->to_string(); \
        } \
    }
    #define LOG_MAP_IN_VALUE(var, prefix) {\
        if(var == nullptr) {\
            SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "var " << #var << " is null"; \
        } \
        else{ \
            auto v = var->get_value(); \
            for(auto& i : v){   \
                SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << #prefix \
                << " " << #var << " value = {" << i.first << " : " << i.second << "}"; \
            }   \
        } \
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

    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/test.yml");
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

#endif

#if 0
class Person {
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;
    bool operator== (const Person& other) const {
        return m_name == other.m_name && m_age == other.m_age && m_sex == other.m_sex;
    }
};
namespace sylar {

template<>
class Converter<std::string, Person> {
public:
    static Person convert(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};
template<>
class Converter<Person, std::string> {
public:
    static std::string convert(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

sylar::ConfigVar<Person>::ptr g_person =
    sylar::ConfigMgr.look_up("class.person", Person(), "system person");

sylar::ConfigVar<std::map<std::string, Person> >::ptr g_person_map =
    sylar::ConfigMgr.look_up("class.map", std::map<std::string, Person>(), "system person");

sylar::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map =
    sylar::ConfigMgr.look_up("class.vec_map", std::map<std::string, std::vector<Person> >(), "system person");

void test_class() {
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_person "
        << "before:\n" << g_person->to_string();
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_person_map "
        << "before: size = " << g_person_map->get_value().size() << '\n' << g_person_map->to_string();
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_person_vec_map "
        << "before: size = " << g_person_vec_map->get_value().size() << '\n' << g_person_vec_map->to_string();

    g_person->add_listener([](const Person& old_value, const Person& new_value){
        SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG)
            << "g_person value changed from \"" << old_value.m_name << "\"," << old_value.m_age << "," << old_value.m_sex
            << " to \"" << new_value.m_name << "\"," << new_value.m_age << "," << new_value.m_sex;
    });

    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/test.yml");
    sylar::ConfigMgr.load_from_yaml(root);

    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG)
        << "after:\n" << g_person->to_string();
    //XX_PM(g_person_map, "class.map after");
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_person_vec_map "
        << "after: size = " << g_person_map->get_value().size() << '\n' << g_person_map->to_string();
    SYLAR_LOG(sylar::LoggerMgr.get_root(), sylar::LogLevel::Level::DEBUG) << "g_person_vec_map "
        << "after: size = " << g_person_vec_map->get_value().size() << '\n' << g_person_vec_map->to_string();
}
#endif

void test_load_logger(){

    // auto root_logger = sylar::LoggerMgr.get_root();
    // SYLAR_LOG(root_logger, sylar::LogLevel::Level::DEBUG)
    //     << "Before load logger config: DEBUG log message";
    // SYLAR_LOG(root_logger, sylar::LogLevel::Level::INFO)
    //     << "Before load logger config: INFO log message";
    std::cout << sylar::LoggerMgr.to_YAML_string() << std::endl;


    YAML::Node root = YAML::LoadFile("/home/liamu/sylar-lin/bin/conf/log.yml");
    sylar::ConfigMgr.load_from_yaml(root);
    
    std::cout << sylar::LoggerMgr.to_YAML_string() << std::endl;
    auto system_log = sylar::LoggerMgr.get_logger("system");
    SYLAR_LOG(system_log, sylar::LogLevel::Level::INFO) << "hello system" << std::endl;
    // SYLAR_LOG(root_logger, sylar::LogLevel::Level::DEBUG)
    //     << "After load logger config: DEBUG log message";
    // SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::DEBUG)
    //     << "This is system logger: DEBUG log message";
    // SYLAR_LOG(root_logger, sylar::LogLevel::Level::INFO)
    //     << "After load logger config: INFO log message";
    // SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
    //     << "This is system logger: INFO log message";
    // SYLAR_LOG(root_logger, sylar::LogLevel::Level::ERROR)
    //     << "After load logger config: ERROR log message";
    // SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::ERROR)
    //     << "This is system logger: ERROR log message";
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

    //test_config();
    //test_class();
    test_load_logger();

    return 0;
}