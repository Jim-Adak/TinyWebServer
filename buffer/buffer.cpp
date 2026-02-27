//
// Created by Adak Jim on 2026/2/27.
//


#include "buffer.h"


//读写下标初始化，vector<char>初始化
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize),readPos_(0),writePos_(0) {}

//可写的数量，buffer大小 - 写下标
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}


