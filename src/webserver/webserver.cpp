//
// Created by Adak Jim on 2026/4/27.
//

#include "webserver.h"

using namespace std;

WebServer::WebServer( int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
            timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    srcDir_ = getcwd(nullptr,256);
    assert(srcDir_);
    strcat(srcDir_,"/resources/");
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;

    //初始化操作
    SqlConnPool::Instance()->Init("localhost",sqlPort,sqlUser,sqlPwd,dbName,connPoolNum); //连接池单例的初始化
    //初始化事件和初始化socket（监听）
    InitEventMode_(trigMode);
    if (!InitSocket_()) { isClose_ = true; }

    // 是否打开日志标志
    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP; //检测socket关闭
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP; //EPOLLONESHOT由一个线程处理
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent_ != EPOLLET;
            break;
        case 2:
            listenEvent_ != EPOLLET;
            break;
        case 3:
            listenEvent_ != EPOLLET;
            connEvent_ != EPOLLET;
            break;
        default:
            listenEvent_ != EPOLLET;
            connEvent_ !=EPOLLET;
            break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}
