#ifndef __SYLAR_SINGLETON_H__
#define __SYLAR_SINGLETON_H__

namespace sylar{

template <typename T>
class Singleton{
public:
    static T& Instance(){
        static T t;
        return t;
    }
};

#define Singleton_Constructor(T) \
private:\
    T();\
    T(const T&) = delete;\
    T& operator=(const T&) = delete;\
    friend class Singleton<T>;



}




#endif