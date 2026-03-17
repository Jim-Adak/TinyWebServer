//
// Created by Confuse on 2026/3/17.
//

#include "../Log/Log.h"
#include"../Pool/ThreadPool.h"
//Linux下将以下代码开放
// #include <features.h>
//
// #if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
// #include <sys/syscall.h>
// #define gettid() syscall(SYS_gettid)
// #endif