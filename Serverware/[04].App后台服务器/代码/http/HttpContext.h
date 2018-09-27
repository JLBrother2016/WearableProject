#ifndef MUDUO_NET_HTTP_HTTPCONTEXT_H
#define MUDUO_NET_HTTP_HTTPCONTEXT_H

#include <muduo/base/copyable.h>

#include "HttpRequest.h"

namespace muduo
{
namespace net
{

class Buffer;

class HttpContext : public muduo::copyable {
public:
    // 解析请求的状态
    enum HttpRequestParseState {
        kExpectRequestLine, // 正在解析起始行
        kExpectHeaders,     // 正在解析首部
        kExpectBody,        // 正在解析主体
        kGotAll,            // 解析完成
    };

    HttpContext() : state_(kExpectRequestLine) {}

    bool parseRequest(Buffer* buf, Timestamp receiveTime);

    bool gotAll() const { return state_ == kGotAll; }

    void reset() // 重置
    {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& request() const { return request_; } // 返回Http请求

    HttpRequest& request() { return request_; } // 返回Http请求

private:

    bool processRequestLine(const char* begin, const char* end);   // 解析起始行

    HttpRequestParseState     state_;     // 解析状态机的状态
    HttpRequest               request_;   // 客户端的请求
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCONTEXT_H
