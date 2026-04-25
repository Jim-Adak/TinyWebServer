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

class HeapTimer {
public:
    HeapTimer(){ heap_.reserve(64); } //保留（扩充）容量
    ~HeapTimer() { clear(); }
    void adjust(int id,int newExpires);
    void add(int id,int timeOut,const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();
private:
    void del_(size_t i);
    void siftup_(size_t i);
    bool siftdown_(size_t i,size_t n);
    void SwapNode_(size_t i,size_t j);
    std::vector<TimerNode> heap_;
    //key : id value : vector的下标
    std::unordered_map<int,size_t> ref_; //id对应的在heap_中的下标，方便用heap_的时候查找
};

#endif //TINYWEBSERVE_HEAPTIMER_H