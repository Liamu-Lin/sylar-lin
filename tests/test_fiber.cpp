#include "fiber.h"
#include <iostream>

void func(void* args){
    std::cout << "In fiber func" << std::endl;
    for(int i = 0; i < 5; ++i){
        std::cout << "Fiber iteration " << i << std::endl;
        sylar::Fiber::fiber_yield();
    }    
}

struct arg_struct{
    int a;
    std::string b;
};
void func2(void* args){
    arg_struct* arg = static_cast<arg_struct*>(args);
    std::cout << "In fiber func2" << std::endl;
    std::cout << "Fiber func2 args: " << arg->a << ", " << arg->b << std::endl;
    for(int i = 0; i < 5; ++i){
        std::cout << "Fiber func2 iteration " << i << std::endl;
        sylar::Fiber::fiber_yield();
    }
}

void test_fiber(){
    std::shared_ptr<sylar::Fiber> fiber(new sylar::Fiber(&func, nullptr));
    fiber->fiber_resume();
    for(int i = 0; i < 5; ++i){
        std::cout << "Main iteration " << i << std::endl;
        fiber->fiber_resume();
    }
    fiber->fiber_resume(); // resume after termination
    std::cout << "Fiber state: " << static_cast<int>(fiber->get_state()) << std::endl;
    
    arg_struct arg;
    arg.a = 42;
    arg.b = "Fiber2 argument";
    std::shared_ptr<sylar::Fiber> fiber2(new sylar::Fiber(&func2, &arg));
    fiber2->fiber_resume();
    for(int i = 0; i < 5; ++i){
        std::cout << "Main iteration for fiber2 " << i << std::endl;
        fiber2->fiber_resume();
    }

}

int main(){

    test_fiber();

    return 0;
}