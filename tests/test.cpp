#include <iostream>
#include "log.h"
#include <unistd.h>
#include <ctime>

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
    sylar::Logger::ptr logger(new sylar::Logger("TestLogger"));

    sylar::LogAppender::ptr stdout_appender(new sylar::StdoutLogAppender);
    stdout_appender->set_level(sylar::LogLevel::Level::DEBUG);

    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d{%m-%d %Y %H:%M:%S}%n[%p] %c >>>%n%r%nIn file: %f:%l %nContent: %m%ntest %%%n>>> end log%n"));
    stdout_appender->set_formatter(fmt);
    logger->add_appender(stdout_appender);
    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, clock(), 0, 0, time(0)));
    event->set_content("Hello sylar log");
    logger->log(sylar::LogLevel::Level::DEBUG, event);

    // logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    // sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    // sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    // file_appender->setFormatter(fmt);
    // file_appender->setLevel(sylar::LogLevel::ERROR);

    // logger->addAppender(file_appender);

    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0)));
    // event->getSS() << "hello sylar log";
    // logger->log(sylar::LogLevel::DEBUG, event);
    


    return 0;
}