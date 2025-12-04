#include "scheduler.h"
#include <iostream>
#include <vector>

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
    sleep(2);
    scheduler->stop();

    SYLAR_LOG(sylar::LoggerMgr.get_logger("system"), sylar::LogLevel::Level::INFO)
        << "test_scheduler finished";

}

int main(){

    test_scheduler();

    return 0;
}