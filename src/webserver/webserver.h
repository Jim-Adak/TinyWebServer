//
// Created by Adak Jim on 2026/4/27.
//

#ifndef TINYWEBSERVER_WEBSERVER_H
#define TINYWEBSERVER_WEBSERVER_H

#include<unordered_map>
#include<fcntl.h>
#include<unistd.h>
#include<assert.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include"../epoll/epoller.h"
#include"../heaptimer/heaptimer.h"
#include"../log/Log.h"
#include"../pool/sqlconnpool.h"
#include"../pool/ThreadPool.h"
#include"../http/httpconn.h"

class WebServer {
private:
    int port_;
    bool openLinger_;
    int timeoutMS_; //毫秒MS
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_; //监听事件
    uint32_t connEvent_; //连接事件

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users_;
};

#endif //TINYWEBSERVER_WEBSERVER_H