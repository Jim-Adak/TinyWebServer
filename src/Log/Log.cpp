//
// Created by Confuse on 2026/3/9.
//

#include "Log.h"

//构造函数
Log::Log() {
    fp = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

