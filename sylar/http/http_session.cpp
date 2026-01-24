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
    size_t parsed = 0, read_len = 0;
    bool finished = false;
    do{
        try{
            read_len += read(&data[read_len], buff_size - read_len);
            finished = parser->execute(data, read_len, parsed);
            if(parser->has_error() || read_len >= buff_size)
                return nullptr;
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
        if(read_len - parsed >= body_length)
            memcpy(&body[0], data + parsed, body_length);
        else{
            size_t readed = read_len - parsed;
            memcpy(&body[0], data + parsed, readed);
            try{
                read_fix_length(&body[readed], body_length - readed);
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