//
// Created by Adak Jim on 2026/4/22.
//

#ifndef TINYWEBSERVER_HEAPTIMER_H
#define TINYWEBSERVER_HEAPTIMER_H
#include<queue>
#include<unordered_map>
#include<time.h>
#include<arpa/inet.h>
#include<algorithm>
#include<functional>
#include<assert.h>
#include<chrono>
#include"../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires; //超时时间点
    TimeoutCallBack cb; //回调function<void()>
    bool operator< (const TimerNode &t) { //重载大于运算符
        return expires < t.expires;
    }
    bool operator > (const TimerNode &t) { //重载小于运算符
        return expires > t.expires;
    }
};

#endif //TINYWEBSERVER_HEAPTIMER_H