//
// Created by Confuse on 2026/3/15.
//

#include"sqlconnpool.h"


SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool pool;
    return &pool;
}