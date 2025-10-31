#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <stdint.h>
#include <string>
#include <memory>

#include <vector>
#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

namespace sylar{

class LogLevel{
public:
    enum Level{
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const std::string to_string(Level level);
    static Level from_string(const std::string& str);
};

//log entry
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    std::string get_file_name() const { return file_name_; }
    int32_t get_line() const { return line_; }
    uint32_t get_elapse() const { return elapse_; }
    uint64_t get_time() const { return time_; }
    uint32_t get_fiber_id() const { return fiber_id_; }
    uint32_t get_thread_id() const { return thread_id_; }
    std::string get_content() const { return content_; }

private:
    std::string file_name_;
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

    LogFormatter(const std::string& pattern);

    bool init_items();

    //format the event to to specified format and output to os
    void format(std::ostream& os, Logger& logger, LogLevel level, LogEvent::ptr event);

private:
    class FormatterItem{
    public:
        typedef std::shared_ptr<FormatterItem> ptr;
        //safe when delete derived class
        virtual ~FormatterItem();

        //format the item to to specified format and output to os
        void virtual format(std::ostream& os, LogEvent::ptr event) = 0;
    };

private:
    std::string pattern_;
    std::vector<FormatterItem::ptr> items_;
};

// log output location
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    //safe to delete derived classes using base class pointer
    virtual ~LogAppender();

    //force derived class to implement this method
    //only ouput logs whose level is higher than level_
    virtual void log(Logger& logger, LogLevel level, LogEvent::ptr event) = 0;

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

    LogLevel get_level() const { return level_; }
    void set_level(LogLevel level) { level_ = level; }

private:
    std::string name_;
    LogLevel level_;    //output logs that meet this level
    std::list<LogAppender::ptr> appenders_;

};

// log appender for stdout
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(Logger& logger, LogLevel level, LogEvent::ptr event) override;

private:

};

// log appender for file
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& file_name);

    void log(Logger& logger, LogLevel level, LogEvent::ptr event) override;

    //return true if reopen successfully
    bool reopen();

private:
    std::string file_name_;
    std::ofstream file_ostream_;
};


}

#endif