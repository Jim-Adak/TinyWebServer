//
// Created by Adak Jim on 2026/4/20.
//

#ifndef TINYWEBSERVER_HTTPCONN_H
#define TINYWEBSERVER_HTTPCONN_H

#include<sys/types.h>
#include<sys/uio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>

#include"../Log/Log.h"
#include"../buffer/buffer.h"
#include"httprequest.h"
#include"httpresponse.h"

//进行读写数据并调用httpreequest 来解析数据以及httpresponse来生成响应

class HttpConn {

};


#endif //TINYWEBSERVER_HTTPCONN_H
