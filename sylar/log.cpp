#include "log.h"

constexpr static sylar::LogLevel::Level ROOT_LEVEL = sylar::LogLevel::Level::DEBUG;

namespace sylar{

class StringFormatterItem : public LogFormatter::FormatterItem{
public:
    StringFormatterItem(const std::string& str): str_(str){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << str_;
    }
private:
    std::string str_;
};
class MessageFormatterItem : public LogFormatter::FormatterItem{
public:
    MessageFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_content();
    }
};
class LevelFormatterItem : public LogFormatter::FormatterItem{
public:
    LevelFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << LogLevel::to_string(level);
    }
};
class ElapseFormatterItem : public LogFormatter::FormatterItem{
public:
    ElapseFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_elapse();
    }
};
class NameFormatterItem : public LogFormatter::FormatterItem{
public:
    NameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << logger->get_name();
    }
};
class ThreadIdFormatterItem : public LogFormatter::FormatterItem{
public:
    ThreadIdFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_thread_id();
    }
};
class FiberIdFormatterItem : public LogFormatter::FormatterItem{
public:
    FiberIdFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_fiber_id();
    }
};
class ThreadNameFormatterItem : public LogFormatter::FormatterItem{
public:
    ThreadNameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_thread_name();
    }
};
class FileNameFormatterItem : public LogFormatter::FormatterItem{
public:
    FileNameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_file_name();
    }
};
class DateTimeFormatterItem : public LogFormatter::FormatterItem{
public:
    DateTimeFormatterItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :format_(format){
        if(format_.empty())
            format_ = "%Y-%m-%d %H:%M:%S";
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        struct tm tm;
        time_t time = event->get_time();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format_.c_str(), &tm);
        os << buf;
    }
private:
    std::string format_;
};
class LineFormatterItem : public LogFormatter::FormatterItem{
public:
    LineFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << event->get_line();;
    }
};
class NewLineFormatterItem : public LogFormatter::FormatterItem{
public:
    NewLineFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << std::endl;
    }
};
class TabFormatterItem : public LogFormatter::FormatterItem{
public:
    TabFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override{
        os << "\t";
    } 
};

Logger::Logger(const std::string& name, LogLevel::Level level):
    name_(name), level_(level){
    ;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level < level_)
        return;
    auto self = shared_from_this();
    if(appenders_.empty())
        LoggerMgr.get_root_appender()->log(self, level, event);
    else{
        for(auto& appender: appenders_)
            appender->log(self, level, event);
    }
}
void Logger::debug(LogEvent::ptr event){
    log(LogLevel::Level::DEBUG, event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::Level::INFO, event);
}
void Logger::warn(LogEvent::ptr event){
    log(LogLevel::Level::WARN, event);
}
void Logger::error(LogEvent::ptr event){
    log(LogLevel::Level::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::Level::FATAL, event);
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
void Logger::clear_appenders(){
    appenders_.clear();
}

//LoggerManager
LoggerManager::LoggerManager(){
    root_.reset(new Logger("root", ROOT_LEVEL));
    root_appender_.reset(new StdoutLogAppender);
    root_->add_appender(root_appender_);
    loggers_["root"] = root_;
}

std::shared_ptr<Logger> LoggerManager::get_logger(const std::string& name){
    auto ret = loggers_.find(name);
    if(ret != loggers_.end())
        return ret->second;

    std::shared_ptr<Logger> logger(new Logger(name));
    loggers_[name] = logger;
    return logger;
}

bool LoggerManager::add_logger(const std::string& name, sylar::LogLevel::Level level){
    auto it = loggers_.find(name);
    if(it != loggers_.end())
        return false;
    loggers_[name] = std::shared_ptr<Logger>(new Logger(name, level));
    return true;
}
bool LoggerManager::del_logger(const std::string& name){
    if(name == "root")
        return false;
    return loggers_.erase(name);
}


//LogLevel
const std::string LogLevel::to_string(Level level){
    switch(level){
        case(Level::DEBUG):
            return "DEBUG";
        case(Level::INFO):
            return "INFO";
        case(Level::WARN):
            return "WARN";
        case(Level::ERROR):
            return "ERROR";
        case(Level::FATAL):
            return "FATAL";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
LogLevel::Level LogLevel::from_string(const std::string& str){
    if(str == "DEBUG" || str == "debug")
        return Level::DEBUG;
    else if(str == "INFO" || str == "info")
        return Level::INFO;
    else if(str == "WARN" || str == "warn")
        return Level::WARN;
    else if(str == "ERROR" || str == "error")
        return Level::ERROR;
    else if(str == "FATAL" || str == "fatal")
        return Level::FATAL;
    return Level::UNKNOWN;
}


//LogEvent
LogEvent::LogEvent(const std::string& file_name, int32_t line, uint32_t elapse,
             pid_t thread_id, fid_t fiber_id, uint64_t time)
    :file_name_(file_name),
    line_(line),
    elapse_(elapse),
    fiber_id_(fiber_id),
    thread_id_(thread_id),
    time_(time){
        //thread_name_ = sylar::Thread::GetName();
}

bool LogEvent::set_content(const std::string& fmt, ...){
    va_list al;
    va_start(al, fmt);
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt.c_str(), al);
    if(len != -1){
        ss_ << std::string(buf, len);
        free(buf);
    }
    va_end(al);
    return true;
}

//LogEventWrap
LogEventWrap::LogEventWrap(std::shared_ptr<Logger> logger, std::shared_ptr<LogEvent> event, LogLevel::Level level)
    :logger_(logger),
    event_(event),
    level_(level){
    ;
}

LogEventWrap::~LogEventWrap(){
    logger_->log(level_, event_);
}

std::stringstream& LogEventWrap::get_ss(){
    return event_->get_ss();
}

//LogAppender
LogAppender::LogAppender(){
    formatter_.reset(new LogFormatter);
}
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
void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level < level_)
        return;
    if(formatter_)
        formatter_->format(std::cout, logger, level, event);
}


//FileLogAppender
FileLogAppender::FileLogAppender(const std::string& file_name)
    :file_name_(file_name){
    file_ostream_.open(file_name_);
    if(file_ostream_)
        std::cout << "Open File success.\n" << file_name << '\n';
    else
        std::cout << "Open File fail.\n" << file_name << '\n';
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level < level_)
        return;
    if(formatter_)
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
    legal_pattern_ = init_items();
}

bool LogFormatter::init_items(){
    bool legal_pattern = true;
    std::size_t len = pattern_.length();
    std::string normal_str;
    for(std::size_t idx = 0; idx < len; ++idx){
        //handle normal string or %%
        if(pattern_[idx] != '%')
            normal_str.append(1, pattern_[idx]);
        else if(idx + 1 < len && pattern_[idx + 1] == '%'){
            normal_str.append(1, '%');
            idx += 1;
        }
        //handle format items
        else{
            char item_type = 0;
            std::string format_str;
            //check wthether the item is legal
            idx += 1;
            if(idx >= len || !FormatterItem::is_legal_item(pattern_[idx])){
                legal_pattern = false;
                break;
            }
            item_type = pattern_[idx];
            //check whether the format is legal
            if(idx + 1 < len && pattern_[idx+1] == '{'){
                idx += 1;
                while(++idx < len && pattern_[idx] != '}')
                    format_str.append(1, pattern_[idx]);
                if(pattern_[idx] != '}'){
                    legal_pattern = false;
                    break;
                }
            }
            //create corresponding FormatterItem
            if(!normal_str.empty()){
                items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem(normal_str)));
                normal_str.clear();
            }
            switch(item_type){
                case 'm':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new MessageFormatterItem("")));
                    break;
                case 'p':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new LevelFormatterItem("")));
                    break;
                case 'r':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new ElapseFormatterItem("")));
                    break;
                case 'c':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new NameFormatterItem("")));
                    break;
                case 't':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new ThreadIdFormatterItem("")));
                    break;
                case 'F':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new FiberIdFormatterItem("")));
                    break;
                case 'f':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new FileNameFormatterItem("")));
                    break;
                case 'N':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new ThreadNameFormatterItem("")));
                    break;
                case 'd':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new DateTimeFormatterItem(format_str)));
                    break;
                case 'l':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new LineFormatterItem("")));
                    break;
                case 'n':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new NewLineFormatterItem("")));
                    break;
                case 'T':
                    items_.push_back(LogFormatter::FormatterItem::ptr(new TabFormatterItem("")));
                    break;
            }

        }
    }
    if(legal_pattern == false){
        items_.clear();
        items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem("<<pattern error>>\n")));
        return false;
    }
    else{
        if(!normal_str.empty())
            items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem(normal_str)));
        return true;
    }
}

void LogFormatter::format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event){
    for(auto& item: items_)
        item->format(os, logger, level, event);
}


//FormatterItem
LogFormatter::FormatterItem::~FormatterItem(){
    ;
}

bool LogFormatter::FormatterItem::is_legal_item(char c){
    switch(c){
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
            return true;
        default:
            return false;
    }
    return false;
}




}