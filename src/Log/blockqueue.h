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
public:
    explicit BlockQueue(size_t maxsize = 1000);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T& item);
    void push_front(const T& item);
    bool pop(T& item); //弹出的任务放item
    bool opo(T& item,int timeout); //等待时间
    void clear();
    T front();
    T back();
    size_t capacity();
    sieze_t size();

    void flush();
    void Close();
private:
    deque<T> deq_; //底层数据结构
    mutex mtx_; //锁
    bool isClose_; //关闭标志
    size_t capacity_; //容量
    condition_variable condConsumer_; //消费者条件变量
    condition_variable condProducer_; //生产者条件变量
};

template<typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacity_(maxsize) {
    assert(maxsize > 0);
    isClose_ = false;
}

template<typename T>
BlockQueue<T>::~BlockQueue() {
    Close();
}

template<typename T>
void BlockQueue<T>::Close() {
    std::lock_guard<mutex> locker(mtx_); //操控队列之前，都需要上锁
    deq_.clear(); //清空队列
    clear();
    isClose_ = true;
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

template<typename T>
void BlockQueue<T>::clear() {
    std::lock_guard<mutex> locker(mtx_);
    deq_.clear();
}

template<typename T>
bool BlockQueue<T>::empty() {
    std::lock_guard<mutex> locker(mtx_);
    return deq_.empty();
}

template<typename T>
bool BlockQueue<T>::full() {
    std::lock_guard<mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<typename T>
void BlockQueue<T>::push_back(const T &item) {
    std::unique_lock<mutex> locker(mtx_);
    while (deq_.size()>=capacity_) { //队列满了，需要等待
        condProducer_.wait(locker); //暂停生产，等待消费者唤醒生产条件变量
    }
    deq_.push_back(item);
    condConsumer_.notify_one(); //唤醒消费者
}

template<typename T>
void BlockQueue<T>::push_front(const T &item) {
    std::unique_lock<mutex> locker(mtx_);
    while (deq_.size()>=capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}






#endif //TINYWEBSERVER_BLOCKQUEUE_H