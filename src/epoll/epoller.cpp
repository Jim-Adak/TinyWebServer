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


