//
// Created by Confuse on 2026/3/22.
//

#ifndef TINYWEBSERVER_HTTPREQUEST_H
#define TINYWEBSERVER_HTTPREQUEST_H
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<regex>
#include<errno.h>
#include<mysql.h>
#include"../buffer/buffer.h"
#include"../Log/Log.h"
#include"../Pool/sqlconnpool.h"

class HttpRequest {
private:
    bool ParseRequest(const std::string& line); //处理请求行
    void ParseHeader(const std::string& line); //处理请求头
    void ParseBody_(const std::string& line); //处理请求体
};

#endif //TINYWEBSERVER_HTTPREQUEST_H