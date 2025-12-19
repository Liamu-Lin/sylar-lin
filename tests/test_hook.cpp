#include "iomanager.h"
#include "hook.h"
#include "log.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

void test_sleep() {
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(2, "iomanager_test"));

    iom->add_fiber([](void*){
        sleep(2);
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sleep 2";
    });

    iom->add_fiber([](void*){
        sleep(3);
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
                << "sleep 3";
    });
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
            << "test_sleep";
}

int main(){
    test_sleep();
    return 0;
}