#ifndef __SYLAR_TYPE_H__
#define __SYLAR_TYPE_H__

#include <string>
#include <string.h>

namespace sylar{

typedef int pid_t;
typedef int fid_t;

struct CaseInsensitiveLess{
    bool operator()(const std::string& lhs, const std::string& rhs) const{
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

}


#endif