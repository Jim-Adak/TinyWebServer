//
// Created by Adak Jim on 2026/2/27.
//

#ifndef BUFFER_H
#define BUFFER_H
#include<iostream>
#include<cstring> //prepare
#include<unistd.h> //write
#include<sys/uio.h>
#include<vector> //readv
#include<atomic>
#include<assert.h>


class Buffer {
private:
    char* BeginPtr_(); //buffer开头
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<size_t> readPos_; //读的下标
    std::atomic<size_t> writePos_; //写的下标
};