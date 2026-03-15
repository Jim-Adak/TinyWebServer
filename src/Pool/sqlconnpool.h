//
// Created by Confuse on 2026/3/14.
//

#ifndef TINYWEBSERVER_SQLCONNPOOL_H
#define TINYWEBSERVER_SQLCONNPOOL_H

#include<mysql.h>
#include<string>
#include<queue>
#include<mutex>
#include<semaphore.h>
#include<thread>
#include"../Log/Log.h"

class SqlConnPool {
public:
    static SqlConnPool* Instance();

    MYSQL* GetConn();
    void Init(const char* host,int port,
              const char* user,const char* pwd,
              const char* dbName,int connSize);
    void ClosePool();
private:
    SqlConnPool() = default;
    ~SqlConnPool() = default;

    int MAX_CONN_;
    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};


#endif //TINYWEBSERVER_SQLCONNPOOL_H