#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <iostream>
#include <map>
#include <inttypes.h>
#include <muduo/base/FileUtil.h>

using namespace muduo;
using namespace muduo::net;

void doGetMethod(const HttpRequest& req, HttpResponse* &resp)
{
    std::cout << "do GET method" << std::endl;
    bool ok = true;

    string path = req.path();
    string result;
    int64_t size = 0;
    int err = FileUtil::readFile(path.c_str(), 32*1024, &result, &size);

    if (err) {
        ok = false;
    }

    resp->setBody(result);
    std::cout << result << std::endl;

    if (ok) {
        resp->setStatusCode(HttpResponse::k200Ok); // 返回码
        resp->setStatusMessage("OK");
        resp->addHeader("Server", "GuJun");
    } else {
        resp->setStatusCode(HttpResponse::k400BadRequest); // 返回码
        resp->setStatusMessage("Not Found");
    }   
}   

void doPostMethod(const HttpRequest& req, HttpResponse* &resp)
{
    std::cout << "do POST method" << std::endl;
    bool ok = true;

    string fileName = req.getHeader("File-Name");
    if (!fileName.empty()) {
        FileUtil::AppendFile appendFile(fileName);
        appendFile.append(req.body().c_str(), req.body().size());
        appendFile.flush();
        std::cout << fileName << "upload success~" << std::endl;
    } else {
        ok = false;
    }

    if (ok) {
        resp->setStatusCode(HttpResponse::k200Ok); // 返回码
        resp->setStatusMessage("OK");
        resp->addHeader("Server", "GuJun");
    } else {
        resp->setStatusCode(HttpResponse::k400BadRequest); // 返回码
        resp->setStatusMessage("Not Found");
    }
}

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    std::cout << "Method: " << req.methodString() << std::endl;
    std::cout << "URL: " << req.path() << std::endl;

    const std::map<string, string>& headers = req.headers();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    std::cout << "Body: \n" << req.body() << std::endl;

    // 设置响应
    switch (req.method()) {
        case HttpRequest::kGet:
            doGetMethod(req, resp);
            break;
        case HttpRequest::kPost:
            doPostMethod(req, resp);
            break;
        case HttpRequest::kHead:
            break;
        case HttpRequest::kPut:
            break;
        case HttpRequest::kDelete:
            break;
        case HttpRequest::kInvalid:
            break;
        default : 
            break;
    }
}

int main(int argc, char const *argv[])
{
    int numThreads = 0;
    if (argc > 1) {
        Logger::setLogLevel(Logger::WARN);
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    HttpServer server(&loop, InetAddress(2017), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}