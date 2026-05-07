//
// Created by Adak Jim on 2026/4/20.
//

#ifndef TINYWEBSERVER_HTTPCONN_H
#define TINYWEBSERVER_HTTPCONN_H

#include<sys/types.h>
#include<sys/uio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>

#include"../Log/Log.h"
#include"../buffer/buffer.h"
#include"httprequest.h"
#include"httpresponse.h"

//进行读写数据并调用httpreequest 来解析数据以及httpresponse来生成响应

class HttpConn {
public:
    HttpConn();
    ~HttpConn();
    void init(int sockFd,const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void Close();
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;
    bool process();

    //写的总长度
    int ToWriteBytes() {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuff_; //读缓冲区
    Buffer writeBuff_;//写缓冲区

    HttpRequest request_;
    HttpResponse response_;
};


#endif //TINYWEBSERVER_HTTPCONN_H
