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

#endif //TINYWEBSERVER_HEAPTIMER_H