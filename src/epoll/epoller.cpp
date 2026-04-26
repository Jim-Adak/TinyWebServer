//
// Created by Adak Jim on 2026/4/26.
//

#include "epoller.h"

Epoller::Epoller(int maxEvent) : epollFd_(epoll_create(512)),events_(maxEvent) {
    assert(epollFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollFd_);
}

bool Epoller::AddFd(int fd,uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_,EPOLL_CTL_ADD,fd,&ev);
}
