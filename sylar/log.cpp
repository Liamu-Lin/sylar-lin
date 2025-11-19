#include "log.h"
#include "config.h"

constexpr static sylar::LogLevel::Level ROOT_LEVEL = sylar::LogLevel::Level::DEBUG;

namespace sylar{

struct LogAppenderEntry{
    int type_; //0: stdout 1: file
    std::string file_; //when type_ == 1
    LogLevel::Level level_ = LogLevel::Level::DEBUG;
    std::string formatter_ = "";
};
struct LoggerEntry{
    std::string name_;
    LogLevel::Level level_ = LogLevel::Level::DEBUG;
    std::list<LogAppenderEntry> appenders_;
    bool operator< (const LoggerEntry& oth) const {
        return name_ < oth.name_;
    }
    bool operator== (const LoggerEntry& oth) const {
        return name_ == oth.name_;
    }
};

template<>
class Converter<std::string, LoggerEntry>{
public:
    static LoggerEntry convert(const std::string& v){
        LoggerEntry logger;
        YAML::Node node = YAML::Load(v);
        logger.name_ = node["name"].as<std::string>();
        if(node["level"].IsDefined())
            logger.level_ = LogLevel::from_string(node["level"].as<std::string>());
        if(node["appenders"].IsDefined()){
            for(size_t x = 0; x < node["appenders"].size(); ++x) {
                auto node_a = node["appenders"][x];
                if(!node_a["type"].IsDefined()) {
                    std::cout << "log config error: appender type is null, " << node_a << std::endl;
                    continue;
                }
                std::string type = node_a["type"].as<std::string>();
                LogAppenderEntry appender;
                if(type == "FileLogAppender") {
                    appender.type_ = 1;
                    if(!node_a["file"].IsDefined()) {
                        std::cout << "log config error: fileappender file is null, " << node_a << std::endl;
                        continue;
                    }
                    appender.file_ = node_a["file"].as<std::string>();
                }
                else if(type == "StdoutLogAppender")
                    appender.type_ = 0;
                else {
                    std::cout << "log config error: appender type is invalid, " << node_a
                              << std::endl;
                    continue;
                }
                if(node_a["formatter"].IsDefined())
                    appender.formatter_ = node_a["formatter"].as<std::string>();
                logger.appenders_.push_back(appender);
            }
        }
        return logger;
    }
};
template<>
class Converter<LoggerEntry, std::string>{
public:
    static std::string convert(const LoggerEntry& logger){
        YAML::Node node;
        node["name"] = logger.name_;
        node["level"] = LogLevel::to_string(logger.level_);

        for(auto& a : logger.appenders_) {
            YAML::Node na;
            if(a.type_ == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = a.file_;
            }
            else if(a.type_ == 0) {
                na["type"] = "StdoutLogAppender";
            }
            na["level"] = LogLevel::to_string(a.level_);
            na["formatter"] = a.formatter_;

            node["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

struct LogIniter{
    LogIniter(){
        auto log_configs = ConfigMgr.look_up("logs", std::set<LoggerEntry>(), "logs config");
        log_configs->add_listener([](const std::set<LoggerEntry>& old_value, const std::set<LoggerEntry>& new_value){
            SYLAR_LOG(LoggerMgr.get_root(), LogLevel::Level::INFO) << "logs config changed";
            for(auto& i : new_value){
                auto logger = LoggerMgr.get_logger(i.name_);
                logger->set_level(i.level_);
                logger->clear_appenders();
                for(auto& a : i.appenders_){
                    LogAppender::ptr appender;
                    if(a.type_ == 0)
                        appender.reset(new StdoutLogAppender);
                    else if(a.type_ == 1)
                        appender.reset(new FileLogAppender(a.file_));
                    if(appender){
                        appender->set_level(a.level_);
                        appender->set_formatter(a.formatter_);
                        logger->add_appender(appender);
                    }
                }
            }
            
            //delete old logger
            for(auto& i : old_value){
                auto it = new_value.find(i);
                if(it != new_value.end())
                    LoggerMgr.del_logger(i.name_);
            }

        });
    }
};
static LogIniter __log_initer;

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


//Logger
Logger::Logger(const std::string& name, LogLevel::Level level):
    name_(name), level_(level){
    ;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level < level_)
        return;
    auto self = shared_from_this();
    appenders_mutex_.lock();
    if(appenders_.empty()){
        appenders_mutex_.unlock();
        LoggerMgr.get_root_appender()->log(self, level, event);
    }
    else{
        for(auto& appender: appenders_)
            appender->log(self, level, event);
        appenders_mutex_.unlock();
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

void Logger::add_appender(LogAppender::ptr appender){
    Mutex::Lock lock(appenders_mutex_);
    appenders_.push_back(appender);
}
bool Logger::del_appender(LogAppender::ptr appender){
    Mutex::Lock lock(appenders_mutex_);
    for(auto it = appenders_.begin(); it != appenders_.end(); ++it){
        if(*it == appender){
            appenders_.erase(it);
            return true;
        }
    }
    return false;
}
void Logger::clear_appenders(){
    Mutex::Lock lock(appenders_mutex_);
    appenders_.clear();
}

std::string Logger::to_YAML_string(){
    YAML::Node node;
    node["name"] = name_;
    node["level"] = LogLevel::to_string(level_);
    {
        Mutex::Lock lock(appenders_mutex_);
        for(auto& appender : appenders_){
            YAML::Node na;
            na = YAML::Load(appender->to_YAML_string());
            node["appenders"].push_back(na);
        }
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

//LoggerManager
LoggerManager::LoggerManager(){
    root_.reset(new Logger("root", ROOT_LEVEL));
    root_appender_.reset(new StdoutLogAppender(ROOT_LEVEL));
    root_->add_appender(root_appender_);
    loggers_["root"] = root_;
}

std::shared_ptr<Logger> LoggerManager::get_logger(const std::string& name){
    Mutex::Lock lock(loggers_mutex_);
    auto ret = loggers_.find(name);
    if(ret != loggers_.end())
        return ret->second;

    std::shared_ptr<Logger> logger(new Logger(name));
    loggers_[name] = logger;
    return logger;
}

bool LoggerManager::add_logger(const std::string& name, sylar::LogLevel::Level level){
    Mutex::Lock lock(loggers_mutex_);
    auto it = loggers_.find(name);
    if(it != loggers_.end())
        return false;
    loggers_[name] = std::shared_ptr<Logger>(new Logger(name, level));
    return true;
}
bool LoggerManager::del_logger(const std::string& name){
    if(name == "root")
        return false;
    auto logger = get_logger(name);
    logger->set_level(LogLevel::Level::UNUSED);
    logger->clear_appenders();
    Mutex::Lock lock(loggers_mutex_);
    return loggers_.erase(name);
}

std::string LoggerManager::to_YAML_string(){
    YAML::Node node;
    {
        Mutex::Lock lock(loggers_mutex_);
        for(auto& i : loggers_)
            node.push_back(YAML::Load(i.second->to_YAML_string()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
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
        case(Level::UNUSED):
            return "UNUSED";
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
    else if(str == "UNUSED" || str == "unused")
        return Level::UNUSED;
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

std::string LogEvent::get_content(){
    Mutex::Lock lock(ss_mutex_);
    return ss_.str();
}
bool LogEvent::set_content(const std::string& fmt, ...){
    va_list al;
    va_start(al, fmt);
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt.c_str(), al);
    if(len != -1){
        Mutex::Lock lock(ss_mutex_);
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
LogAppender::LogAppender(LogLevel::Level level, const std::string& pattern)
    :level_(level){
    formatter_.reset(new LogFormatter(pattern));
}
LogAppender::~LogAppender(){
    ;
}

bool LogAppender::set_formatter(LogFormatter::ptr formatter){
    if(formatter->is_legal_pattern()){
        Mutex::Lock lock(formatter_mutex_);
        formatter_ = formatter;
        return true;
    }
    else{
        std::cerr << "Set formatter failed, illegal pattern: " << formatter->get_pattern() << std::endl;
        return false;
    }
}
bool LogAppender::set_formatter(const std::string& pattern){
    LogFormatter::ptr formatter(new LogFormatter(pattern));
    return set_formatter(formatter);
}

LogFormatter::ptr LogAppender::get_formatter(){
    Mutex::Lock lock(formatter_mutex_);
    return formatter_;
}


//StdoutLogAppender
void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level < level_)
        return;
    Mutex::Lock lock(formatter_mutex_);
    formatter_->format(std::cout, logger, level, event);
}

std::string StdoutLogAppender::to_YAML_string() {
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    node["level"] = LogLevel::to_string(level_);
    {
        Mutex::Lock lock(formatter_mutex_);
        if(formatter_)
            node["formatter"] = formatter_->get_pattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


//FileLogAppender
FileLogAppender::FileLogAppender(const std::string& file_name)
    :file_name_(file_name){
    FSUtil::open_for_write(ofstream_, file_name_);
}
FileLogAppender::FileLogAppender(const std::string& file_name, LogLevel::Level level, const std::string& pattern)
    :LogAppender(level, pattern)
    ,file_name_(file_name){
    FSUtil::open_for_write(ofstream_, file_name_);
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level < level_)
        return;
    uint64_t now = event->get_time();
    if(now - last_time_ > 100 && !FSUtil::file_or_dir_exists(file_name_)){
        if(!reopen()){
            std::cerr << "FileLogAppender::log reopen file " << file_name_ << " failed" << std::endl;
            return;
        }
    }
    last_time_ = now;
    Mutex::Lock ofs_lock(ofs_mutex_);
    Mutex::Lock formatter_lock(formatter_mutex_);
    formatter_->format(ofstream_, logger, level, event);
}

bool FileLogAppender::reopen(){
    Mutex::Lock ofs_lock(ofs_mutex_);
    ofstream_.close();
    FSUtil::open_for_write(ofstream_, file_name_, std::ios_base::out | std::ios_base::app);
    return (bool)ofstream_;
}

std::string FileLogAppender::to_YAML_string(){
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = file_name_;
    node["level"] = LogLevel::to_string(level_);
    {
        Mutex::Lock lock(formatter_mutex_);
        node["formatter"] = formatter_->get_pattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

//LogFormatter
LogFormatter::LogFormatter(const std::string& pattern)
    :pattern_(pattern){
    if(pattern_.empty())
        pattern_ = "%d{%Y-%m-%d %H:%M:%S} [%p][%c] thread:%t %N fiber:%F %f:%l%T%m%n";
    legal_pattern_ = init_items();
    if(!legal_pattern_)
        pattern_.clear();
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
        return false;
    }
    else{
        if(!normal_str.empty())
            items_.push_back(LogFormatter::FormatterItem::ptr(new StringFormatterItem(normal_str)));
        return true;
    }
}

void LogFormatter::format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event){
    Mutex::Lock lock(items_mutex_);
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