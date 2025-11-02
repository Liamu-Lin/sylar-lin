#include "util.h"

namespace sylar{

pid_t get_thread_id(){
    return std::this_thread::get_id();
}

fid_t get_fiber_id(){
    return 666;
}






}