//
// Created by Confuse on 2026/3/9.
//

#include "Log.h"

//构造函数
Log::Log() {
    fp_ = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

Log::~Log() {
    while (!deque_->empty()) {
        deque_->flush(); //唤醒消费者，处理掉剩下的任务
    }
    deque_->Close(); //关闭队列
    writeThread_->join(); //等待当前线程完成手中的任务
    if (fp_) { //冲洗文件缓冲区，关闭文件描述符
        std::lock_guard<std::mutex> locker(mtx_); //
        flush(); //清空缓冲区中的数据
        fclose(fp_); //关闭日志文件
    }
}