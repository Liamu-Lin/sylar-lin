#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include "util.h"
#include "log.h"

static sylar::Logger::ptr assert_logger = sylar::LoggerMgr.get_logger("system");

#if defined __GNUC__ || defined __llvm__
    #define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
    #define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
    #define SYLAR_LIKELY(x)       (x)
    #define SYLAR_UNLIKELY(x)     (x)
#endif 

#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))){\
        SYLAR_LOG(assert_logger, sylar::LogLevel::Level::DEBUG) << "ASSERT: "#x" Failed!" \
            << "\nBacktrace:\n"<< sylar::backtrace_symbols(100, 2, "    "); \
        abort(); \
    }
#define SYLAR_ASSERT_M(x, w) \
    if(SYLAR_UNLIKELY(!(x))){\
        SYLAR_LOG(assert_logger, sylar::LogLevel::Level::DEBUG) << "ASSERT: "#x" Failed! " << w \
            << "\nBacktrace:\n"<< sylar::backtrace_symbols(100, 2, "    "); \
        abort(); \
    }





#endif