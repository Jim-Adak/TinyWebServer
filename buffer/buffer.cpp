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

const char* Buffer::Peek() const {
    return &buffer_[readPos_];
}

//确保可写的长度
void Buffer::EnsureWriteable(size_t len) {
    if (len>WritableBytes()) {
        MakeSpace_(len);
    }
    assert(len<=WritableBytes());
}

//移动写下标，在Append中使用
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
}

//读取len长度,移动写下标
void Buffer::Retrieve(size_t len) {
    readPos_ +=len;
}

//读取到end位置
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek()<=end);
    Retrieve(end - Peek()); //end指针 - 读指针长度
}

//取出所有数据，buffer归零，读下标归零，在别的函数中用到
void Buffer::RetrieveAll() {
    bzero(&buffer_[0],buffer_.size()); //覆盖原本数据
    readPos_ = writePos_ =0;
}

//取出剩余可读的str
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(),ReadableBytes());
    RetrieveAll();
    return str;
}

//写指针的位置
const char *Buffer::BeginWriteConst() const {
    return &buffer_[writePos_];
}

char *Buffer::BeginWrite() {
    return &buffer_[writePos_];
}

//添加str到缓冲区
void Buffer::Append(const char* str,size_t len) {
    assert(str);
    EnsureWriteable(len);//确保可写的长度
    std::copy(str,str+len,BeginWrite()); //将str放到写下标开始的地方
    HasWritten(len); //移动写下标
}









