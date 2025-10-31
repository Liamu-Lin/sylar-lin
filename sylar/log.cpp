#include "log.h"

namespace sylar{



Logger::Logger(const std::string name)
    :name_(name) {
    ;
}

void Logger::log(LogLevel level, LogEvent::ptr event){
    if(level < level_)
        return;
    for(auto& appender: appenders_)
        appender->log(level, event);
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

LogLevel Logger::get_level() const {
    return level_;
}
void Logger::set_level(LogLevel level) {
    level_ = level;
}


//LogAppender
void LogAppender::set_formatter(LogFormatter::ptr formatter){
    formatter_ = formatter;
}
LogFormatter::ptr LogAppender::get_formatter() const{
    return formatter_;
}

//StdoutLogAppender
void StdoutLogAppender::log(LogLevel level, LogEvent::ptr event) {
    if(level < level_)
        return;
    std::cout << formatter_->format(event);
}


//FileLogAppender
FileLogAppender::FileLogAppender(const std::string& file_name)
    :file_name_(file_name){

}

void FileLogAppender::log(LogLevel level, LogEvent::ptr event) {
    if(level < level_)
        return;
    file_ostream_ << formatter_->format(event);
}

bool FileLogAppender::reopen(){
    if(file_ostream_.is_open())
        file_ostream_.close();
    file_ostream_.open(file_name_);
    return (bool)file_ostream_;
}


}