#include "http_connection.h"
#include "http_parser.h"

namespace sylar{
namespace http{

static sylar::Logger::ptr g_logger = sylar::LoggerMgr.get_logger("system");

HttpConnection::HttpConnection(Socket::ptr sock)
    :SocketStream(sock){
}

HttpResponse::ptr HttpConnection::recv_response(){
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buff_size = HttpResponseParser::get_response_buffer_size();
    std::shared_ptr<char> buffer(new char[buff_size+1], [](char* ptr){delete[] ptr;});
    size_t unparsed = 0;
    char* data = buffer.get();
    bool finished = false;
    do{
        try{
            size_t parsed = 0;
            size_t read_len = read(&data[unparsed], buff_size - unparsed) + unparsed;
            data[read_len] = '\0';
            finished = parser->execute(data, read_len, parsed);
            // delete parsed data
            unparsed = read_len - parsed;
            memmove(data, data + parsed, unparsed);
            if(parser->has_error() || unparsed >= buff_size)
                throw std::runtime_error("Response parser error or exceed max buffer size");
        }catch(std::runtime_error& e){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "HttpConnection::recv_response read error " << e.what();
            return nullptr;
        } catch(...){
            SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpConnection::recv_response read error";
            return nullptr;
        }
    } while(!finished);

    std::string body;
    // chunked body
    if(parser->is_chunked()){
        read_chunked_body(body, data, buff_size, unparsed);
        // unparsed = 0;
        // do{
        //     bool one_chunk_finished = false;
        //     // parser chunked headers
        //     do{
        //         try{
        //             size_t parsed = 0;
        //             size_t read_len = read(&data[unparsed], buff_size - unparsed) + unparsed;
        //             data[read_len] = '\0';
        //             one_chunk_finished = parser->execute_chunked(data, read_len, parsed);
        //             // delete parsed data
        //             unparsed = read_len - parsed;
        //             memmove(data, data + parsed, unparsed);
        //             if(parser->has_error() || unparsed >= buff_size)
        //                 throw std::runtime_error("Response chunked parser error or exceed max buffer size");
        //         } catch(std::runtime_error& e){
        //             SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "HttpConnection::recv_response read chunked body error " << e.what();
        //             return nullptr;
        //         } catch(...){
        //             SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpConnection::recv_response read chunked body error";
        //             return nullptr;
        //         }
        //     } while(!one_chunk_finished);
        //     // load chunked body
        //     uint64_t chunk_size = parser->get_content_length();
        //     if(chunk_size <= unparsed){
        //         body.append(data, chunk_size);
        //         // delete used data
        //         unparsed -= chunk_size;
        //         memmove(data, data + chunk_size, unparsed);
        //     } else{
        //         body.append(data, unparsed);
        //         chunk_size -= unparsed;
        //         unparsed = 0;
        //         while(chunk_size > 0){
        //             try{
        //                 size_t to_read = std::min(buff_size, chunk_size);
        //                 size_t read_len = read(data, to_read);
        //                 body.append(data, read_len);
        //                 chunk_size -= read_len;
        //             } catch(std::runtime_error& e){
        //                 SYLAR_LOG(g_logger, sylar::LogLevel::Level::INFO) << "HttpConnection::recv_response read chunked body error " << e.what();
        //                 return nullptr;
        //             } catch(...){
        //                 SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpConnection::recv_response read chunked body error";
        //                 return nullptr;
        //             }
        //         }
        //     }
        // } while(!parser->is_chunk_done());
    }
    // unchunked body
    else{
        uint64_t body_length = parser->get_content_length();
        if(body_length){
            body.resize(body_length);
            if(unparsed >= body_length)
                memcpy(&body[0], data, body_length);
            else{
                memcpy(&body[0], data, unparsed);
                try{
                    read_fix_length(&body[unparsed], body_length - unparsed);
                } catch(...){
                    SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpConnection::recv_response read body error";
                    return nullptr;
                }
            }
        }
    }
    if(!body.empty()){
        parser->get_response()->set_body(body);
    }
    return parser->get_response();
}


void HttpConnection::send_request(HttpRequest::ptr rsp){
    std::stringstream ss;
    ss << *rsp;
    std::string str = ss.str();
    try{
        write_fix_length(str.c_str(), str.size());
    } catch(...){
        SYLAR_LOG(g_logger, sylar::LogLevel::Level::WARN) << "HttpConnection::send_request write error";
    }
}


}
}