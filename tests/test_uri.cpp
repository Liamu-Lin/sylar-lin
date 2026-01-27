#include "uri.h"
#include "log.h"
#include "iomanager.h"

sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

void test_uri(void* args = nullptr){
    //sylar::URI::ptr uri = sylar::URI::create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    sylar::URI::ptr uri = sylar::URI::create("http://admin@www.baidu.com/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    //sylar::URI::ptr uri = sylar::URI::create("http://admin@www.baidu.com");
    //sylar::URI::ptr uri = sylar::URI::create("http://www.sylar.top/test/uri");
    std::cout << *uri << std::endl;
    auto addr = uri->get_address();
    std::cout << *addr << std::endl;
}

int main(){
    std::shared_ptr<sylar::IOManager> iom(new sylar::IOManager(2, "iomanager_test"));

    iom->add_fiber(test_uri);
    iom->start();

    return 0;
}