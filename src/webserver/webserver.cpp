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

    //是否打开日志的标志
    if (openLog) {
        Log::Instance()->init(logLevel,"./log",".log",logQueSize);
        if (isClose_) { LOG_ERROR("========== Server init error!===========");}
        else {
            LOG_INFO("============ Server init ===============");
            LOG_INFO("Port:&d,OpenLinger:%",port_,OptLinger ? "true":"false");
        }
    }
}


