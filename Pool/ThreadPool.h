//
// Created by Confuse on 2026/3/11.
//

#ifndef TINYWEBSERVER_THREADPOOL_H
#define TINYWEBSERVER_THREADPOOL_H

#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<thread>
#include<assert.h>

class ThreadPool {
public:
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    //尽量使用make_shared代替new，如果通过new再传递给shared_ptr，内存是不连续的，会造成内存碎片化
    explicit ThreadPool(int ThreadCount = 8) : pool_(std::make_shared<Pool>()) {  //make_shared传递右值，功能是在动态内存中分配一个对象并初始化它，返回指向此对象的shared_ptr

    }
private:
    //用一个结构体封装起来，方便调用
    struct Pool {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>>tasks; //任务队列，函数类型为void()
    };
    std::shared_ptr<Pool> pool_;
};


#endif //TINYWEBSERVER_THREADPOOL_H