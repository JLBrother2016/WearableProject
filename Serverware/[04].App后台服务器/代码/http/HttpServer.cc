// 按照TcpServer接收连接->处理连接这个顺序来看，首先将调用onConnection，
// 该函数为Tcp连接创建Http上下文变量HttpContext，这个类主要用于接收客户请求（这里为Http请求），并解析请求。

#include <muduo/net/http/HttpServer.h>

#include <muduo/base/Logging.h>

#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

namespace muduo
{
namespace net
{
namespace detail
{

// 默认Http回调返回错误码，默认返回404 Not Found!
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true); // 主动关闭TCP连接
}

}
}
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(boost::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(boost::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer() {}

void HttpServer::start()
{
    LOG_WARN << "HttpServer[" << server_.name() << "] starts listenning on " << server_.ipPort();
    server_.start();
}

// 新连接回调
void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        conn->setContext(HttpContext());
    }
}

// 消息回调
void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());
    // 解析请求
    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll()) { // 请求已经解析完毕
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const string& connection = req.getHeader("Connection");
    bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);     // 构造响应
    httpCallback_(req, &response);    // 用户回调
    Buffer buf;
    response.appendToBuffer(&buf);    // 此时response已经构造好，将向客户发送Response添加到buffer中
    conn->send(&buf);                 // 发送响应
    if (response.closeConnection()) { // 如果非Keep-Alive则直接关掉
        conn->shutdown();
    }
}

