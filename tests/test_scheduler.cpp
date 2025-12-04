#include "scheduler.h"
#include "log.h"
#include <iostream>
#include <vector>

std::shared_ptr<sylar::Logger> g_logger = sylar::LoggerMgr.get_logger("system");

struct arg_struct{
    int a;
    std::string b;
};

void fun1(void* args){
    arg_struct* arg = (static_cast<arg_struct*>(args));
    for(int i = 0; i < 5; ++i){
        std::cout << "fun1: " << arg->a << ", " << arg->b << ", " << i << std::endl;
        //sylar::Fiber::fiber_yield();
    }
}


void test_scheduler(){
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "start test_scheduler";

    std::shared_ptr<sylar::Scheduler> scheduler(new sylar::Scheduler(3, "test_scheduler"));
    std::vector<std::shared_ptr<sylar::Fiber>> fibers;
    for(int i = 1; i <= 5; ++i){
        arg_struct* arg = new arg_struct();
        arg->a = i;
        arg->b = "hello_scheduler" + std::to_string(i);
        std::shared_ptr<sylar::Fiber> fiber(new sylar::Fiber(fun1, arg));
        fibers.push_back(fiber);
        scheduler->add_fiber(fiber);
    }
    scheduler->start();
    //sleep(2);
    scheduler->stop();

    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "test_scheduler finished";
}


void test_fiber(void*) {
    static int s_count = 5;
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->add_fiber(&test_fiber, nullptr, sylar::get_thread_id());
    }
}

void test_scheduler_recursive(){
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "test_scheduler_recursive";

    std::shared_ptr<sylar::Scheduler> sc(new sylar::Scheduler(3, "test_scheduler_recursive"));
    sc->start();
    sleep(2);
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "schedule";
    sc->add_fiber(&test_fiber);
    sc->stop();
    SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO)
        << "over";

}


int main(){

    //test_scheduler();

    test_scheduler_recursive();

    return 0;
}