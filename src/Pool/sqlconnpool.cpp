//
// Created by Confuse on 2026/3/15.
//

#include"sqlconnpool.h"


SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool pool;
    return &pool;
}

void SqlConnPool::Init(const char* host,int port,
              const char* user,const char* pwd,
              const char* dbName,int connSize = 10) {
    assert(connSize > 0);
    for (int i=0;i<connSize;i++) {
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);
        if (!conn) {
            LOG_ERROR("MySql init error!");
            assert(conn);
        }
        conn = mysql_real_connect(conn,host,user,pwd,dbName,port,nullptr,0);
        if (!conn) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.emplace(conn);
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_,0,MAX_CONN_);
}

