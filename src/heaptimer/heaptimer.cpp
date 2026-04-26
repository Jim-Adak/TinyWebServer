//
// Created by Adak Jim on 2026/4/25.
//

#include"heaptimer.h"

void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id] = i;  //结点内部id所在索引位置也要变化
    ref_[heap_[j].id] = j;
}

void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i<heap_.size());
    size_t parent = (i-1) / 2;
    while(parent >= 0) {
        if (heap_[parent] > heap_[i]) {
            SwapNode_(i,parent);
            i = parent;
            parent = (i - 1) / 2;
        }else {
            break;
        }
    }
}

//false:不需要下滑 true：下滑成功
bool HeapTimer::siftdown_(size_t i, size_t n) {
    assert(i >= 0 && i< heap_.size());
    assert(n >= 0 && n<=heap_.size());
    auto index = i;
    auto child = 2*index+1;
    while (child < n) {
        if (child + 1 < n && heap_[child+1] < heap_[child]) {
            child++;
        }
    }
}





