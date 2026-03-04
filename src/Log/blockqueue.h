//
// Created by Adak Jim on 2026/3/4.
//

#ifndef TINYWEBSERVER_BLOCKQUEUE_H
#define TINYWEBSERVER_BLOCKQUEUE_H
#include<deque>
#include<condition_variable>
#include<mutex>
#include<sys/time.h>

template<typename T>
class BlockQueue {
private:
    deque<T> deq_; //底层数据结构
    mutex mtx_; //锁
    bool isClose_; //关闭标志
    size_t capacity; //容量
    condition_variable condConsumer_; //消费者条件变量
    condition_variable condProducer_; //生产者条件变量
};

#endif //TINYWEBSERVER_BLOCKQUEUE_H