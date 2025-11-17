#include "thread.h"
#include "log.h"
#include "mutex.h"
#include <iostream>
#include <vector>

volatile int global_count = 0;
sylar::Mutex g_mutex;
sylar::RWMutex g_rwmutex;
sylar::Semaphore g_sem(1);

void fun(){
    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "name: " << sylar::Thread::get_name()
        << " id: " << sylar::get_thread_id();
    
    for(int i = 0; i < 100000; ++i){
        //sylar::Mutex::Lock lock(g_mutex);
        //sylar::RWMutex::WLock lock(g_rwmutex);
        //sylar::RWMutex::RLock lock(g_rwmutex);
        g_sem.Wait();
        ++global_count;
        g_sem.Post();
    }
    
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

    std::cout << "global_count = " << global_count << std::endl;

    return 0;
}