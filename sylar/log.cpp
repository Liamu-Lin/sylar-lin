#include "log.h"

namespace sylar{

class StringFormatterItem : public LogFormatter::FormatterItem{
public:
    StringFormatterItem(const std::string& str): str_(str){;}
    void format(std::ostream& os, LogEvent::ptr event) override{
        os << str_;
    }
private:
    std::string str_;
}
class MessageFormatterItem : public LogFormatter::FormatterItem{
public:
    MessageFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, LogEvent::ptr event) override{
        os << event->get_content();
    }
}
class LevelFormatterItem : public LogFormatter::FormatterItem{
public:
    LevelFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, LogEvent::ptr event) override{
        os << event->get_();
    }
}


Logger::Logger(const std::string name)
    :name_(name) {
    ;
}

void Logger::log(LogLevel level, LogEvent::ptr event){
    if(level < level_)
        return;
    for(auto& appender: appenders_)
        appender->log(*this, level, event);
}
void Logger::deubg(LogEvent::ptr event){
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event){
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event){
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::FATAL, event);
}

//not thread-safe now
void Logger::add_appender(LogAppender::ptr appender){
    appenders_.push_back(appender);
}
bool Logger::del_appender(LogAppender::ptr appender){
    for(auto it = appenders_.begin(); it != appenders_.end(); ++it){
        if(*it == appender){
            appenders_.erase(it);
            return true;
        }
    }
    return false;
}


//LogLevel
static const std::string LogLevel::to_string(Level level){
    switch(level){
        case(DEBUG):
            return "DEBUG";
        case(INFO):
            return "INFO";
        case(WARN):
            return "WARN";
        case(ERROR):
            return "ERROR";
        case(FATAL):
            return "FATAL";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
static LogLevel::Level LogLevel::from_string(const std::string& str){
    if(str == "DEBUG" || str == "debug")
        return DEBUG;
    else if(str == "INFO" || str == "info")
        return INFO;
    else if(str == "WARN" || str == "warn")
        return WARN;
    else if(str == "ERROR" || str == "error")
        return ERROR;
    else if(str == "FATAL" || str == "fatal")
        return FATAL;
    return UNKNOWN;
}


//LogEvent



//LogAppender
LogAppender::~LogAppender(){
    ;
}

void LogAppender::set_formatter(LogFormatter::ptr formatter){
    formatter_ = formatter;
}
LogFormatter::ptr LogAppender::get_formatter() const{
    return formatter_;
}

//StdoutLogAppender
void StdoutLogAppender::log(Log& logger, LogLevel level, LogEvent::ptr event) {
    if(level < level_)
        return;
    formatter_->format(std::cout, logger, level, event);
}


//FileLogAppender
FileLogAppender::FileLogAppender(const std::string& file_name)
    :file_name_(file_name){

}

void FileLogAppender::log(Log& logger, LogLevel level, LogEvent::ptr event) {
    if(level < level_)
        return;
    formatter_->format(file_ostream_, logger, level, event);
}

bool FileLogAppender::reopen(){
    if(file_ostream_.is_open())
        file_ostream_.close();
    file_ostream_.open(file_name_);
    return (bool)file_ostream_;
}

//LogFormatter
LogFormatter::LogFormatter(const std::string& pattern)
    :pattern_(pattern){
    ;
}

bool LogFormatter::init_items(){
    ;
}

void LogFormatter::format(std::ostream& os, Logger& logger, LogLevel level, LogEvent::ptr event){
    for(auto& item: items_)
        item->format(os, event);
}




//FormatterItem
LogFormatter::FormatterItem::~FormatterItem(){
    ;
}




}