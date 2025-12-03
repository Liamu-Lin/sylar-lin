#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

#include "log.h"
#include "util.h"
#include "mutex.h"

#include <pthread.h>

#include <memory>
#include <string>
#include <functional>

namespace sylar{

class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
public:
    Thread(std::function<void()> cb, const std::string& name = "UNKNOWN");
    ~Thread();

    void join();
    
    static Thread* get_this();
    static std::string get_name();
    static pid_t get_id();
    
    static void set_name(const std::string& name);
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    bool operator==(const Thread&) = delete;

    static void* run(void* args);
private:
    pid_t id_;
    std::string name_;
    pthread_t thread_;
    std::function<void()> cb_;
    Semaphore sema_;
};



}

#endif