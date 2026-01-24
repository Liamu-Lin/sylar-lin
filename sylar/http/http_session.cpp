#include "http/http_session.h"
#include "http/http_parser.h"
#include "log.h"
#include <stdexcept>

namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

HttpSession::HttpSession(Socket::ptr sock)
    :SocketStream(sock){
}

HttpRequest::ptr HttpSession::recv_request(){
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::get_request_buffer_size();
    std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){delete[] ptr;});
    char* data = buffer.get();
    bool finished = false;
    size_t unparsed = 0;
    do{
        try{
            size_t parsed = 0;
            size_t read_len = read(&data[unparsed], buff_size - unparsed) + unparsed;
            finished = parser->execute(data, read_len, parsed);
            // delete parsed data
            unparsed = read_len - parsed;
            memmove(data, data + parsed, unparsed);
            if(parser->has_error() || unparsed >= buff_size)
                throw std::runtime_error("Request parser error or exceed max buffer size");
        }catch(std::runtime_error& e){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "HttpSession::recv_request read error " << e.what();
            return nullptr;
        } catch(...){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpSession::recv_request read error";
            return nullptr;
        }
    } while(!finished);
    uint64_t body_length = parser->get_content_length();
    if(body_length){
        std::string body;
        body.resize(body_length);
        if(unparsed >= body_length)
            memcpy(&body[0], data, body_length);
        else{
            memcpy(&body[0], data, unparsed);
            try{
                read_fix_length(&body[unparsed], body_length - unparsed);
            } catch(...){
                SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpSession::recv_request read body error";
                return nullptr;
            }
        }
        parser->get_request()->set_body(body);
    }
    return parser->get_request();
}

void HttpSession::send_response(HttpResponse::ptr rsp){
    std::stringstream ss;
    ss << *rsp;
    std::string str = ss.str();
    try{
        write_fix_length(str.c_str(), str.size());
    } catch(...){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpSession::send_response write error";
    }
}


}
}