//
// Created by Adak Jim on 2026/4/26.
//

#ifndef TINYWEBSERVER_EPOLLER_H
#define TINYWEBSERVER_EPOLLER_H

#include<sys/epoll.h> //epoll_ctl()
#include<unistd.h> //close()
#include<assert.h>//close(
#include<vector>
#include<errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool AddFd(int fd,uint32_t events);
    bool ModFd(int fd,uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
private:
    int epollFd_;
    std::vector<struct epoll_event> events_;
};

#endif //TINYWEBSERVER_EPOLLER_H