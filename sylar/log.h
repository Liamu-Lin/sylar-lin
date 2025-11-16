#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <stdint.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <memory>
#include <thread>

#include <vector>
#include <list>
#include <map>

#include <iostream>
#include <fstream>
#include <sstream>

#include "util.h"
#include "singleton.h"


#define SYLAR_LOG(logger, level) \
    if(logger->get_level() <= level)  \
        sylar::LogEventWrap(logger, sylar::LogEvent::ptr(new sylar::LogEvent(__FILE__, __LINE__, \
                                clock(), sylar::get_thread_id(), sylar::get_fiber_id(), time(0))), level).get_ss()

namespace sylar{

class Logger;
class LoggerManager;

class LogLevel{
public:
    enum class Level : int {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5,
        UNUSED = 6
    };
    LogLevel() = delete;
    static const std::string to_string(Level level);
    static Level from_string(const std::string& str);
};

//log entry
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(const std::string& file_name, int32_t line, uint32_t elapse,
             pid_t thread_id, fid_t fiber_id, uint64_t time);

    std::string get_file_name() const { return file_name_; }
    int32_t get_line() const { return line_; }
    uint32_t get_elapse() const { return elapse_; }
    uint64_t get_time() const { return time_; }
    uint32_t get_fiber_id() const { return fiber_id_; }
    const std::thread::id& get_thread_id() const { return thread_id_; }
    std::string get_content() const { return ss_.str(); }
    std::string get_thread_name() const { return thread_name_; }
    std::stringstream& get_ss() { return ss_; }

    bool set_content(const std::string& fmt, ...);
private:
    std::string file_name_;
    int32_t line_ = 0;
    uint32_t elapse_ = 0;   //in microsecond
    uint32_t fiber_id_ = 0; 
    std::thread::id thread_id_;
    uint64_t time_ = 0;     //time when the LogEvent was recorded
    std::stringstream ss_;
    std::string thread_name_;
};

//log event wrap
class LogEventWrap{
public:
    LogEventWrap(std::shared_ptr<Logger> logger, std::shared_ptr<LogEvent> event, LogLevel::Level level);
    ~LogEventWrap();

    std::stringstream& get_ss();
private:
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<LogEvent> event_;
    LogLevel::Level level_;
};

// log formatter
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    class FormatterItem{
    public:
        typedef std::shared_ptr<FormatterItem> ptr;
        //safe when delete derived class
        virtual ~FormatterItem();
        //format the item to to specified format and output to os
        void virtual format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        static bool is_legal_item(char c);
    };

    LogFormatter(const std::string& pattern = "%d{%Y-%m-%d %H:%M:%S} [%p][%c] thread:%t %N fiber:%F %f:%l%T%m%n");

    //format the event to to specified format and output to os
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::string get_pattern() const { return pattern_; }
    bool is_legal_pattern() const { return legal_pattern_; }
private:
    bool legal_pattern_;
    std::string pattern_;
    std::vector<FormatterItem::ptr> items_;

    bool init_items();
};

// log output location
class LogAppender{
public:
    typedef std::shared_ptr<LogAppender> ptr;

    LogAppender();
    LogAppender(LogLevel::Level level, const std::string& pattern = "");
    //safe to delete derived classes using base class pointer
    virtual ~LogAppender();

    //force derived class to implement this method
    //only ouput logs whose level is higher than level_
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    bool set_formatter(LogFormatter::ptr formatter);
    bool set_formatter(const std::string& pattern);
    LogFormatter::ptr get_formatter() const { return formatter_; }

    void set_level(LogLevel::Level level) { level_ = level; }
    LogLevel::Level get_level() const { return level_; }

    virtual std::string to_YAML_string() = 0;
protected:
    LogLevel::Level level_;    //output logs that meet this level
    LogFormatter::ptr formatter_;
};

// log outputter
class Logger : public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;
    friend class LoggerManager;

    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void add_appender(LogAppender::ptr appender);
    bool del_appender(LogAppender::ptr appender);
    void clear_appenders();

    void set_level(LogLevel::Level level) { level_ = level; }

    LogLevel::Level get_level() const { return level_; }
    std::string get_name() const { return name_; }

    std::string to_YAML_string();
private:
    Logger() = delete;
    Logger(const Logger&) = delete;
    Logger(const std::string& name, LogLevel::Level level = LogLevel::Level::DEBUG);
    std::string name_;
    LogLevel::Level level_;    //output logs that meet this level
    std::list<LogAppender::ptr> appenders_;
};

//logger manager
class LoggerManager{
Singleton_Constructor(LoggerManager)
public:
    #define LoggerMgr Singleton<sylar::LoggerManager>::Instance()
    std::shared_ptr<Logger> get_root() const { return root_; }
    std::shared_ptr<LogAppender> get_root_appender() const { return root_appender_; }
    std::shared_ptr<Logger> get_logger(const std::string& name);
    std::size_t get_logger_cnt() const { return loggers_.size(); }

    bool add_logger(const std::string& name, LogLevel::Level level =LogLevel::Level::DEBUG);
    bool del_logger(const std::string& name);

    std::string to_YAML_string();
private:
    std::map<std::string, std::shared_ptr<Logger>> loggers_;
    std::shared_ptr<Logger> root_;
    std::shared_ptr<LogAppender> root_appender_;
};

// log appender for stdout
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    StdoutLogAppender() = default;
    StdoutLogAppender(LogLevel::Level level, const std::string& pattern = "")
        :LogAppender(level, pattern){
    }

    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    std::string to_YAML_string() override;
};

// log appender for file
class FileLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& file_name);
    FileLogAppender(const std::string& file_name, LogLevel::Level level, const std::string& pattern = "");

    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    bool reopen();

    std::string to_YAML_string() override;
private:
    std::string file_name_;
    std::ofstream file_ostream_;
};


}

#endif