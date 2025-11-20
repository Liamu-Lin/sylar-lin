#include "macro.h"
#include "log.h"

void fun(){
    SYLAR_ASSERT(true);
    //SYLAR_ASSERT(false);
    SYLAR_ASSERT_M(2 == 3, "2 not equal 3");
}

void test_assert(){
    fun();
}

int main(){
    
    test_assert();

    return 0;
}