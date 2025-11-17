#include "util.h"
#include <thread>

namespace sylar{

pid_t get_thread_id(){
    return syscall(SYS_gettid);
}

fid_t get_fiber_id(){
    return 666;
}






}