#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include "util.h"
#include "log.h"

static sylar::Logger::ptr assert_logger = sylar::LoggerMgr.get_logger("system");

#define SYLAR_ASSERT(x) \
    if(!(x)){\
        SYLAR_LOG(assert_logger, sylar::LogLevel::Level::DEBUG) << "ASSERT: "#x" Failed!" \
            << "\nBacktrace:\n"<< sylar::backtrace_symbols(100, 2, "    "); \
        abort(); \
    }
#define SYLAR_ASSERT_M(x, w) \
    if(!(x)){\
        SYLAR_LOG(assert_logger, sylar::LogLevel::Level::DEBUG) << "ASSERT: "#x" Failed! " << w \
            << "\nBacktrace:\n"<< sylar::backtrace_symbols(100, 2, "    "); \
        abort(); \
    }





#endif