#include "fiber.h"
#include "thread.h"
#include "log.h"

#include <iostream>
#include <vector>
#include <assert.h>

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
    arg_struct* arg = static_cast<arg_struct*>(args);
    sylar::Fiber::fiber_yield();
    int arr[101];
    for(int i = 1; i <= 5; ++i){
        ++(arg->a);
        arr[i] = 114514;
        int b = arr[i];
        std::cout << arg->a << " " << arg->b << " in func1 i = " << i << std::endl;
        sylar::Fiber::fiber_yield();
    }
    std::cout << "end of func in " << arg->b << std::endl;
}
void func2(void*){
    std::cout << "1\n";
    std::cout << "2\n";
    throw std::runtime_error("func2 exception");
    std::cout << "3\n";
}

void test_fiber(void* args){
    arg_struct* arg = static_cast<arg_struct*>(args);
    
    std::shared_ptr<sylar::FiberSharedStackPool> pool(new sylar::FiberSharedStackPool(1, 128 * 1024));
    std::vector<std::shared_ptr<sylar::Fiber>> fibers;
    for(int i = 1; i <= 5; ++i){
        arg_struct* a = new arg_struct;
        a->a = i * 100;
        a->b = arg->b + " shared stack fiber " + std::to_string(i);
        std::shared_ptr<sylar::Fiber> f;
        if(i % 2 == 1)
            f.reset(new sylar::Fiber(&func, a, pool));
        else
            f.reset(new sylar::Fiber(&func1, a, pool));
        fibers.push_back(f);
    }
    for(size_t i = 0; i < fibers.size(); ++i){
        fibers[i]->fiber_resume();
    }
    for(int j = 0; j <= 5; ++j){
        for(size_t i = 0; i < fibers.size(); ++i){
            fibers[i]->fiber_resume();
            //std::cout << "main " << j << ' ' << i << std::endl;
        }
    }
}

void test_fiber_fiber(){
    std::string thread_name = sylar::Thread::get_name();
    SYLAR_LOG(sylar::LoggerMgr.get_logger("sys"), sylar::LogLevel::Level::INFO)
         << "test_fiber_fiber in thread " << thread_name;
    
    
    arg_struct arg;
    arg.b = thread_name;
    std::shared_ptr<sylar::Fiber> fiber(new sylar::Fiber(&test_fiber, &arg));
    fiber->fiber_resume();
    for(int i = 0; i < 5; ++i){
        fiber->fiber_resume();
    }
}

void test_thread_fiber_fiber(){
    std::vector<sylar::Thread::ptr> thrs;
    for(int i = 0; i < 1; ++i){
        sylar::Thread::ptr thr(new sylar::Thread(&test_fiber_fiber, "fiber_thread_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for(auto& thr : thrs)
        thr->join();
}

void test_fiber_reset(){
    arg_struct arg;
    arg.a = 0;
    arg.b = "fiber_reset";

    std::shared_ptr<sylar::Fiber> fiber(new sylar::Fiber(&func, &arg));
    for(int i = 0; i < 3; ++i){
        fiber->fiber_resume();
        std::cout << "main in test_fiber_reset i = " << i << std::endl;
    }

    std::cout << "reset fiber" << std::endl;
    fiber->reset(&func2, nullptr);
    for(int i = 0; i < 7; ++i){
        fiber->fiber_resume();
        std::cout << "main in test_fiber_reset i = " << i << std::endl;
    }

    std::cout << "reset fiber again" << std::endl;
    fiber->reset(&func, &arg);
    for(int i = 0; i < 5; ++i){
        fiber->fiber_resume();
        std::cout << "main in test_fiber_reset i = " << i << std::endl;
    }
}

int main(){
    //test_thread_fiber_fiber();
    struct arg_struct arg;
    arg.b = "main thread";

    test_fiber(&arg);
    test_thread_fiber_fiber();

    test_fiber_reset();


    return 0;
}