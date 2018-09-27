#include <muduo/net/Buffer.h>
#include "HttpContext.h"

using namespace muduo;
using namespace muduo::net;

#include <iostream>

// Http 请求格式
// <method> <request-URL> <version>
// <headers>
//
// <entity-body>

// Http 响应格式
// <version> <staus> <reason-phrase>
// <headers>
//
// <entity-body>


// 解析起始行 格式： <method> <request-URL> <version> 
// 例如 GET /get_data.json HTTP/1.1
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');

    if (space != end && request_.setMethod(start, space)) { // 解析请求方法
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            // 解析URL
            const char* question = std::find(start, space, '?'); // 分割URL 和 Query
            if (question != space) {
                request_.setPath(start, question);  // 设置URL
                request_.setQuery(question, space); // 设置查询 query
            } else {
                request_.setPath(start, space);
            }
            // 解析HTTP版本
            start = space + 1;
            succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
            if (succeed) {
                if (*(end-1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (*(end-1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
  return succeed;
}

// 解析Http请求的状态机
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf); // 解析 start line
                if (ok) {
                    request_.setReceiveTime(receiveTime); // 设置时间
                    buf->retrieveUntil(crlf + 2); // 回收start line buffer
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectHeaders) { // 解析请求头
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':'); // 找到":的位置
                if (colon != crlf) { // 有头部，头部中的每一个都是以CRLF结束的
                    request_.addHeader(buf->peek(), colon, crlf);
                } else { // 首部解析完毕
                    state_ = kExpectBody;
                    hasMore = true;
                }
                buf->retrieveUntil(crlf + 2); // 回收首部
            } else {
              hasMore = false;
            }
        } else if (state_ == kExpectBody) {// 解析主体
            string bodyLength = request_.getHeader("Content-Length"); // 查找header中是否有Content-Length
            if (!bodyLength.empty()) { // 有body
                int len = atoi(bodyLength.c_str());
                if (buf->readableBytes() >= len) { //只有当body部分完全收到Buffer中再来处理
                    const char *start = buf->peek();
                    request_.setBody(start, start+len);
                    state_ = kGotAll; // 全部结束
                    buf->retrieveUntil(start+len); // 回收首部
                }
            } else {
                state_ = kGotAll; // 全部结束
            }
            hasMore = false;
        }
    }
    return ok;
}
