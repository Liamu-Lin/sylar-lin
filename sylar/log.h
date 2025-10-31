#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

namespace sylar{

enum class LogLevel : int{
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

//log entry
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;

private:
    const char* file_name_ = nullptr;
    int32_t line_ = 0;
    uint32_t elapse_ = 0;   //in microsecond
    uint64_t time_ = 0;     //time when the LogEvent was recorded
    uint32_t fiber_id_ = 0; 
    uint32_t thread_id_ = 0;
    std::string content_;
};

// log formatter
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    std::string format(LogEvent::ptr event);    //format the event to to specified format

private:

};

// log output location
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    //safe to delete derived classes using base class pointer
    virtual ~LogAppender() {;}  

    //force derived class to implement this method
    //only ouput logs whose level is higher than level_
    virtual void log(LogLevel level, LogEvent::ptr event) = 0;

    void set_formatter(LogFormatter::ptr formatter);
    LogFormatter::ptr get_formatter() const;

protected:
    LogLevel level_;    //output logs that meet this level
    LogFormatter::ptr formatter_;
};

// log outputter
class Logger{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string name);

    //only ouput logs whose level is higher than level_
    void log(LogLevel level, LogEvent::ptr event);
    void deubg(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void add_appender(LogAppender::ptr appender);
    bool del_appender(LogAppender::ptr appender);

    LogLevel get_level() const;
    void set_level(LogLevel level);

private:
    std::string name_;
    LogLevel level_;    //output logs that meet this level
    std::list<LogAppender::ptr> appenders_;

};

// log appender for stdout
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(LogLevel level, LogEvent::ptr event) override;

private:

};

// log appender for file
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& file_name);

    void log(LogLevel level, LogEvent::ptr event) override;

    //return true if reopen successfully
    bool reopen();

private:
    std::string file_name_;
    std::ofstream file_ostream_;
};


}

#endif