#include "fiber.h"
#include <iostream>

void func(void* args){
    std::cout << "In fiber func" << std::endl;
    for(int i = 0; i < 5; ++i){
        std::cout << "Fiber iteration " << i << std::endl;
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
    std::cout << "Fiber state: " << static_cast<int>(fiber->get_state()) << std::endl;
    
}

int main(){

    test_fiber();

    return 0;
}