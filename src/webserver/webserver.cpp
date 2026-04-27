//
// Created by Adak Jim on 2026/4/27.
//

#include "webserver.h"

using namespace std;

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool PotLinger,
    int sqlPort, const char *sqlUser, const char *sqlPwd, const char *dbName,
    int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize) :
    port_(port),openLinger_(trigMode),timeoutMS_(timeoutMS),isClose_(false),
    timer_(new HeapTimer()),thread_pool_(new ThreadPool(threadNum)),epoller_(new Epoller)
{

}


