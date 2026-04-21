//
// Created by Adak Jim on 2026/4/21.
//

#include"httpconn.h"
using namespace std;

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::init(int fd,const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose() = false;
    LOG_INFO("Client[&d](&s:&d) in, userCount:%d",fd_,GetIP(),GetPort(),(int)userCount);
}

void HttpConn::Close() {
    response_.UnmapFile();
    if (isClose_ == false) {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[&d](&s:&d) in, userCount:%d",fd_,GetIP(),GetPort(),(int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
}



