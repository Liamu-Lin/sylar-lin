#include "thread.h"

static sylar::Logger::ptr thread_logger = sylar::LoggerMgr.get_logger("thread logger");

namespace sylar{

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

Thread* Thread::get_this(){
    return t_thread;
}
std::string Thread::get_name(){
    return t_thread_name;
}
pid_t Thread::get_id(){
    if(t_thread)
        return t_thread->id_;
    else
        return get_thread_id();
}
void Thread::set_name(const std::string& name){
    pthread_t tid = pthread_self();
    pthread_setname_np(tid, name.c_str());
    if(t_thread)
        t_thread->name_ = name;
    t_thread_name = name;
}


Thread::Thread(std::function<void()> cb, const std::string& name) :
    name_(name),
    cb_(cb){
    int rt = pthread_create(&thread_, nullptr, &Thread::run, this);
    if(rt != 0){
        SYLAR_LOG(thread_logger, thread_logger->get_level()) 
            << "pthread_create thread fail, error_id=" << rt << " name=" << name_;
        throw std::logic_error("pthread_create error");
    }
    pthread_setname_np(thread_, name_.c_str());
}
Thread::~Thread(){
    if(thread_){
        pthread_detach(thread_);
    }
}

void Thread::join(){
    if(thread_){
        int rt = pthread_join(thread_, nullptr);
        if(rt != 0){
            SYLAR_LOG(thread_logger, thread_logger->get_level()) 
                << "pthread_join thread fail, error_id=" << rt << " name=" << name_;
        throw std::logic_error("pthread_join error");
        }
        thread_ = 0;
    }
}

void* Thread::run(void* args){
    t_thread = (Thread*)args;
    t_thread->id_ = get_thread_id();
    t_thread_name = t_thread->name_;
    //pthread_setname_np(t_thread->thread_, t_thread->name_.c_str());

    std::function<void()> cb = t_thread->cb_;
    cb.swap(t_thread->cb_);
    cb();

    return nullptr;
}


}