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

//可读的数量，写下标 - 读下标
size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}

//可预留的数量，已经读过的就没用了，等于读下标
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

const char* Buffer::Peak() const {
    return &buffer_[readPos_];
}

//确保可写的长度
void Buffer::ENsureWriteable(size_t len) {
    if (len>WritableBytes()) {
        MakeSpace_(len);
    }
    assert(len<=WritableBytes());
}

//移动写下标，在Append中使用
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
}






