#include "mutex.h"

namespace sylar{

Semaphore::Semaphore(unsigned int count){
    int rt = sem_init(&sem_, 0, count);
    if(rt != 0)
        throw std::logic_error("sem_init error");
}
Semaphore::~Semaphore(){
    sem_destroy(&sem_);
}

void Semaphore::Post(){
    int rt = sem_post(&sem_);
    if(rt != 0)
        throw std::logic_error("sem_post error");
}
void Semaphore::Wait(){
    int rt = sem_wait(&sem_);
    if(rt != 0)
        throw std::logic_error("sem_wait error");
}



}