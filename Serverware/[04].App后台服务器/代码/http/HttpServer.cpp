#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <iostream>
#include <map>

using namespace muduo;
using namespace muduo::net;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    std::cout << "Headers: " << req.methodString() << std::endl;
    std::cout << "URL: " << req.path() << std::endl;
    std::cout << "Version: " << req.getVersion() << std::endl;

    const std::map<string, string>& headers = req.headers();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    std::cout << "Body: \n" << req.body() << std::endl;

    resp->setStatusCode(HttpResponse::k200Ok); // 返回码
    resp->setStatusMessage("OK");
    resp->addHeader("Server", "GuJun");
}

int main(int argc, char const *argv[])
{
    int numThreads = 0;
    if (argc > 1)
    {
      Logger::setLogLevel(Logger::WARN);
      numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    HttpServer server(&loop, InetAddress(80), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}