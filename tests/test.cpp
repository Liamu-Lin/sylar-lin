#include <iostream>
#include "log.h"
#include "util.h"
#include <unistd.h>
#include <ctime>
#include <thread>

/*
case 'm':   //message
        case 'p':   //level
        case 'r':   //elapse
        case 'c':   //logger name
        case 't':   //thread id
        case 'F':   //fiber id
        case 'f':   //file name
        case 'N':   //thread name
        case 'd':   //date time
        case 'l':   //line
        case 'n':   //new line
        case 'T':   //tab
*/

int main(){
    std::cout << "Start test log" << std::endl;
    auto logger = sylar::LoggerMgr.get_logger("TestLogger");

    sylar::LogAppender::ptr stdout_appender(new sylar::StdoutLogAppender);
    //sylar::LogAppender::ptr stdout_appender(new sylar::FileLogAppender("test.log"));
    stdout_appender->set_level(sylar::LogLevel::Level::DEBUG);
    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, clock(), sylar::get_thread_id(), sylar::get_fiber_id(), time(0)));
    event->set_content(">>> This is a test log meassage. <<<");
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d{%m-%d %Y %H:%M:%S}%n[%p] %c >>>%n%r%nIn file: %f:%l %nContent: %m%ntest %%%ntest %%d%ntest%Ttable%ntest threadID: %t%ntest fiberID: %F%n>>> end log%n"));
    stdout_appender->set_formatter(fmt);
    
    
    // logger->add_appender(stdout_appender);

    // logger->log(sylar::LogLevel::Level::DEBUG, event);


    // std::cout << "Start test log macros." << std::endl;
    // SYLAR_LOG(logger, sylar::LogLevel::Level::FATAL) << "test log macros.";
    
    // std::cout << "Start test error formatter." << std::endl;
    // sylar::LogFormatter::ptr efmt(new sylar::LogFormatter("%d{%m-%d %Y %H:%M:%S}%n[%p] %c >>>%n%r%nIn file: %f:%l %vContent: %m%ntest %%%ntest %%d%ntest%Ttable%ntest threadID: %t%ntest fiberID: %F%n>>> end log%n"));
    // stdout_appender->set_formatter(efmt);
    // SYLAR_LOG(logger, sylar::LogLevel::Level::DEBUG) << "test error formatter.";
    
    sylar::LoggerMgr.get_root()->log(sylar::LogLevel::Level::DEBUG, event);
    auto tmp = sylar::LoggerMgr.get_logger("114514");
    tmp->add_appender(stdout_appender);
    tmp->log(sylar::LogLevel::Level::DEBUG, event);
    std::cout << sylar::LoggerMgr.get_logger_cnt();
    sylar::LoggerMgr.del_logger("hahahaha");
    std::cout << sylar::LoggerMgr.get_logger_cnt();
    sylar::LoggerMgr.del_logger("114514");
    std::cout << sylar::LoggerMgr.get_logger_cnt();
    return 0;
}