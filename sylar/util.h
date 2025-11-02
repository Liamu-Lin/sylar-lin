#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <thread>
#include <stdint.h>

namespace sylar{

typedef std::thread::id pid_t;
typedef uint32_t fid_t;

pid_t get_thread_id();
//TODO
fid_t get_fiber_id();






}





#endif