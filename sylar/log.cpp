#include "log.h"

namespace sylar{

class StringFormatterItem : public LogFormatter::FormatterItem{
public:
    StringFormatterItem(const std::string& str): str_(str){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << str_;
    }
private:
    std::string str_;
};
class MessageFormatterItem : public LogFormatter::FormatterItem{
public:
    MessageFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_content();
    }
};
class LevelFormatterItem : public LogFormatter::FormatterItem{
public:
    LevelFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << LogLevel::to_string(level);
    }
};
class ElapseFormatterItem : public LogFormatter::FormatterItem{
public:
    ElapseFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_elapse();
    }
};
class NameFormatterItem : public LogFormatter::FormatterItem{
public:
    NameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << logger->get_name();
    }
};
class ThreadIdFormatterItem : public LogFormatter::FormatterItem{
public:
    ThreadIdFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_thread_id();
    }
};
class FiberIdFormatterItem : public LogFormatter::FormatterItem{
public:
    FiberIdFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_fiber_id();
    }
};
class ThreadNameFormatterItem : public LogFormatter::FormatterItem{
public:
    ThreadNameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_thread_name();
    }
};
class FileNameFormatterItem : public LogFormatter::FormatterItem{
public:
    FileNameFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
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
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
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
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << event->get_line();;
    }
};
class NewLineFormatterItem : public LogFormatter::FormatterItem{
public:
    NewLineFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << std::endl;
    }
};
class TabFormatterItem : public LogFormatter::FormatterItem{
public:
    TabFormatterItem(const std::string& str = ""){;}
    void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override{
        os << "\t";
    } 
};

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
void StdoutLogAppender::log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) {
    if(level < level_)
        return;
    formatter_->format(std::cout, logger, level, event);
}


//FileLogAppender
FileLogAppender::FileLogAppender(const std::string& file_name)
    :file_name_(file_name){

}

void FileLogAppender::log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) {
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
    legal_pattern_ = init_items();
}

bool LogFormatter::init_items(){
    bool legal_pattern = true;
    std::string normal_str;
    for(int idx = 0; idx < pattern_.length(); ++idx){
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
            idx += 1;
            if(idx < len && pattern_[idx] == '{'){
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
        items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem("<<pattern error>>")));
        return false;
    }
    else{
        if(!normal_str.empty())
            items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem(normal_str)));
        return true;
    }
}

void LogFormatter::format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event){
    for(auto& item: items_)
        item->format(os, event);
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