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

//删除指定位置的节点
void HeapTimer::del_(size_t index) {
    assert(index >= 0 && index < heap_.size());
    size_t tmp = index;
    size_t n = heap_.size() - 1;
    assert(tmp <= n);
    //如果就在队尾，就不用移动
    if (index < heap_.size() - 1) {
        SwapNode_(tmp,heap_.size() - 1);
        if (!siftdown_(tmp,n)) {
            siftup_(tmp);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

//调整指定id的节点
void HeapTimer::adjust(int id, int newExpires) {
    assert(heap_.empty() && ref_.count(id));
    heap_[ref_[id]].expires = Clock::now() + MS(newExpires);
}

void HeapTimer::add(int id, int timeOut, const TimeoutCallBack &cb) {
    assert(id>= 0);
    //如果有，则调整
    if (ref_.count(id)) {
        int tmp = ref_[id];
        heap_[tmp].expires = Clock::now() + MS(timeOut);
        heap_[tmp].cb = cb;
        if (!siftdown_(tmp,heap_.size())) {
            siftup_(tmp);
        }
    }else {
        size_t n = heap_.size();
        ref_[id] = n;
        heap_.push_back({id,Clock::now() + MS(timeOut),cb});
        siftup_(n);
    }
}









