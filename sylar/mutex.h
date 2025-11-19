#ifndef __SYLAR_MUTEX_H__
#define __SYLAR_MUTEX_H__

#include "noncopyable.h"

#include <pthread.h>
#include <semaphore.h>

#include <stdexcept>

namespace sylar{

class Semaphore{
public:
    Semaphore(unsigned int count);
    ~Semaphore();

    void Post();
    void Wait();
private:
    sem_t sem_;
};


template<typename T>
class LockGuard{
public:
    LockGuard(T& mutex):
        mutex_(mutex){
        mutex_.lock();
    }
    ~LockGuard(){
        mutex_.unlock();
    }
private:
    T& mutex_;
};
template<typename T>
class ReadLockGuard{
public:
    ReadLockGuard(T& mutex):
        mutex_(mutex){
        mutex_.rlock();
    }
    ~ReadLockGuard(){
        mutex_.unlock();
    }
private:
    T& mutex_;
};
template<typename T>
class WriteLockGuard{
public:
    WriteLockGuard(T& mutex):
        mutex_(mutex){
        mutex_.wlock();
    }
    ~WriteLockGuard(){
        mutex_.unlock();
    }
private:
    T& mutex_;
};

class Mutex : public Noncopyable{
public:
    typedef LockGuard<Mutex> Lock;

    Mutex(){
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~Mutex(){
        pthread_mutex_destroy(&mutex_);
    }

    int lock(){
        return pthread_mutex_lock(&mutex_);
    }
    int unlock(){
        return pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
};

class RWMutex : public Noncopyable{
public:
    typedef ReadLockGuard<RWMutex> RLock;
    typedef WriteLockGuard<RWMutex> WLock;
    RWMutex(){
        pthread_rwlock_init(&rw_mutex_, nullptr);
    }
    ~RWMutex(){
        pthread_rwlock_destroy(&rw_mutex_);
    }
    int rlock(){
        return pthread_rwlock_rdlock(&rw_mutex_);
    }
    int wlock(){
        return pthread_rwlock_wrlock(&rw_mutex_);
    }
    int unlock(){
        return pthread_rwlock_unlock(&rw_mutex_);
    }
private:
    pthread_rwlock_t rw_mutex_;
};

class SpinLock : public Noncopyable{
public:
    typedef LockGuard<SpinLock> Lock;
    SpinLock(){
        pthread_spin_init(&spin_mutex_, 0);
    }
    ~SpinLock(){
        pthread_spin_destroy(&spin_mutex_);
    }
    int lock(){
        return pthread_spin_lock(&spin_mutex_);
    }
    int unlock(){
        return pthread_spin_unlock(&spin_mutex_);
    }
private:
    pthread_spinlock_t spin_mutex_;
};

}

#endif