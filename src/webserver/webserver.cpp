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


void WebServer::Start() {
    int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
    if (!isClose_) { LOG_INFO("======== Server start =========");}
    while (!isClose_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick(); // 获取下一次的超时等待事件(至少这个时间才会有用户过期，每次关闭超时连接则需要有新的请求进来)
        }
        int eventCnt = epoller_->Wait(timeMS);
        for (int i = 0;i < eventCnt;i++) {
            //处理事件
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (fd == listenFd_) {
                DealListen_();
            }else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char *info) {
    assert(fd > 0);
    int ret = send(fd,info,strlen(info),0);
    if (ret < 0) {
        LOG_WARN("send error to client[&d] error!",fd);
    }
    close(fd);
}

void WebServer::CloseConn_(HttpConn *client) {
    assert(client);
    LOG_INFO("Client[%d] quit!",client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd,addr);
    if (timeoutMS_ > 0) {
        timer_->add(fd,timeoutMS_,std::bind(&WebServer::CloseConn_,this,&users_[fd]));
    }
    epoller_->AddFd(fd,EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!:",users_[fd].GetFd());
}

//处理监听套接字，主要逻辑是accept新的套接字，并加入timer和epoller中
void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_,(struct sockaddr*)&addr,&len);
        if (fd <= 0) { return; }
        else if (HttpConn::userCount >= MAX_FD) {
            SendError_(fd,"Server busy!");
            LOG_WARN("Client is full!");
            return;
        }
        AddClient_(fd,addr);
    }while (listenEvent_ & EPOLLET);
}

//处理读事件，主要逻辑是将OnRead加入线程池的任务队列中
void WebServer::DealRead_(HttpConn *client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_,this,client)); //这是一个右值，bind将参数和函数绑定
}

//处理写事件，主要逻辑是将OnWrite加入线程池的任务队列中
void WebServer::DealWrite_(HttpConn *client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_,this,client));
}

void WebServer::ExtentTime_(HttpConn *client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->adjust(client->GetFd(),timeoutMS_);
    }
}










