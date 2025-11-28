#include "fiber.h"
#include "thread.h"
#include "log.h"

#include <iostream>
#include <vector>

struct arg_struct{
    int a;
    std::string b;
};
void func(void* args){
    arg_struct* arg = static_cast<arg_struct*>(args);
    sylar::Fiber::fiber_yield();
    for(int i = 1; i <= 5; ++i){
        ++(arg->a);
        std::cout << arg->a << " " << arg->b << " in func i = " << i << std::endl;
        sylar::Fiber::fiber_yield();
    }
    std::cout << "end of func in " << arg->b << std::endl;
}
void func1(void* args){
    std::cout << "func1 start\n";
    sylar::Fiber::fiber_yield();
    std::cout << "func1 resumed\n";
}

void test_fiber(void* args){
    arg_struct* arg = static_cast<arg_struct*>(args);
    
    std::shared_ptr<sylar::FiberSharedStackPool> pool(new sylar::FiberSharedStackPool(1, 128 * 1024));
    std::vector<std::shared_ptr<sylar::Fiber>> fibers;
    for(int i = 1; i <= 2; ++i){
        arg_struct* a = new arg_struct;
        a->a = i * 100;
        a->b = arg->b + " shared stack fiber " + std::to_string(i);
        std::shared_ptr<sylar::Fiber> f(new sylar::Fiber(&func, a, pool));
        fibers.push_back(f);
    }
    for(size_t i = 0; i < fibers.size(); ++i){
        fibers[i]->fiber_resume();
    }
    for(int j = 0; j <= 5; ++j){
        for(size_t i = 0; i < fibers.size(); ++i){
            fibers[i]->fiber_resume();
            std::cout << "main " << j << ' ' << i << std::endl;
        }
    }

    std::cout << "end of test_fiber in " << std::endl;

}

// void test_fiber_fiber(){
//     std::string thread_name = sylar::Thread::get_name();
//     SYLAR_LOG(sylar::LoggerMgr.get_logger("sys"), sylar::LogLevel::Level::INFO)
//          << "test_fiber_fiber in thread " << thread_name;
    
    
//     arg_struct arg;
//     arg.b = thread_name;
//     std::shared_ptr<sylar::Fiber> fiber(new sylar::Fiber(&test_fiber, &arg));
//     fiber->fiber_resume();
//     for(int i = 0; i < 5; ++i){
//         std::cout << "test_fiber_fiber in " << thread_name << " i = " << i << std::endl;
//         fiber->fiber_resume();
//     }
// }

// void test_thread_fiber_fiber(){
//     std::vector<sylar::Thread::ptr> thrs;
//     for(int i = 0; i < 5; ++i){
//         sylar::Thread::ptr thr(new sylar::Thread(&test_fiber_fiber, "fiber_thread_" + std::to_string(i)));
//         thrs.push_back(thr);
//     }

//     for(auto& thr : thrs)
//         thr->join();
// }

int main(){

    //test_thread_fiber_fiber();
    struct arg_struct arg;
    arg.b = "main thread";

    test_fiber(&arg);


    return 0;
}