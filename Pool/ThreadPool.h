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
        assert(ThreadCount > 0);
        for (int i = 0; i< ThreadCount; i++) {
            std::thread([this]() {
                std::unique_lock<std::mutex> locker(pool_->mtx_);
                while(true) {
                    if (!pool_->tasks.empty()) {
                        auto task = std::move(pool_->tasks.front()); //左值变右值，资产转移
                        pool_->tasks.pop();
                        locker.unlock(); //把任务取出来后提前解锁
                        task();
                        locker.lock(); //又要拿任务了，再次上锁
                    } else if (pool_->isClosed) {
                        break;
                    }else {
                        pool_->cond_.wait(locker); //等待，如果任务来了就notify
                    }
                }
            }).detach();
        }
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