
#include "log.h"
#include "config.h"
#include <iostream>

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::ConfigMgr.look_up<int>("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::ConfigMgr.look_up<float>("system.value", (float)10.2f, "system value");

int main(int argc, char** argv){
    // auto root = sylar::LoggerMgr.get_root();
    
    auto lg = sylar::Logger::ptr(new sylar::Logger("system"));
    lg->set_level(sylar::LogLevel::Level::INFO);
    auto ap = sylar::LogAppender::ptr(new sylar::StdoutLogAppender);
    ap->set_level(sylar::LogLevel::Level::INFO);
    lg->add_appender(ap);

    SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
        << "g_int_value_config=" << g_int_value_config->get_value()
        << " g_float_value_config=" << g_float_value_config->get_value();

    SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
        << "g_int_value_config to_string=" << g_int_value_config->to_string()
        << " g_float_value_config to_string=" << g_float_value_config->to_string();

    g_int_value_config->from_string("1024");
    g_float_value_config->from_string("3.14159");

    SYLAR_LOG(lg, sylar::LogLevel::Level::INFO)
        << "g_int_value_config=" << g_int_value_config->get_value()
        << " g_float_value_config=" << g_float_value_config->get_value();


    return 0;
}