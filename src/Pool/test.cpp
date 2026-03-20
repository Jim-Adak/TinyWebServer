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

void TestLog() {
    int cnt = 0,level = 0;
    Log::Instance()->init(level,"./testlog1",".log",0);
    for (level = 3;level>=0;level--) {
        Log::Instance()->SetLevel(level);
        for (int j = 0;j<10000;j++) {
            for (int i = 0;i<4;i++) {
                LOG_BASE(i,"&s 11111111111 &d ===============","Test",cnt++);
            }
        }
    }
    cnt = 0;
    Log::Instance()->init(level,"./testlog2",".log",5000);
    for (level = 0; level < 4; level++) {
        Log::Instance()->SetLevel(level);
        for (int j = 0; j< 10000; j++) {
            for (int i = 0;i<4; i++) {
                LOG_BASE(i,"&s 2222222 &d ==============","Test",cnt++);
            }
        }
    }
}

void ThreadLogTask(int i,int cnt) {
    Log::Instance()->init(0,"./testThreaddpool",".log",5000);
    ThreadPool threadpool(6);
    for (int i=0;i<18;i++) {
        threadpool.AddTask(std::bind(ThreadLogTask, i % 4,i * 10000));
    }
    getchar();
}