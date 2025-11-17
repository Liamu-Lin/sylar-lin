#include "thread.h"
#include "log.h"
#include <iostream>
#include <vector>

void fun(){
    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "name: " << sylar::Thread::get_name()
        << " this.name: " << sylar::Thread::get_this()->get_name()
        << " id: " << sylar::get_thread_id()
        << " this.id: " << sylar::Thread::get_id();
    sleep(300);
}

void test_thread(){
    std::vector<sylar::Thread::ptr> thrs;    
    for(int i = 0; i < 10; ++i){
        sylar::Thread::ptr thr(new sylar::Thread(&fun, "test_thread_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for(int i = 0; i < 10; ++i)
        thrs[i]->join();

}

int main(){

    test_thread();

    return 0;
}