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

    void ParsePath_(); //处理请求路径
    void ParsePost_(); //处理Post事件
    void ParseFromUrlencoded_(); //从url中解析编码

    static bool UserVerify(const std::string& name,const std::string& pwd,bool isLogin); //用户验证

    PARSE_STATE state_;
    std::string method_,path_,version_,body_;
    std::unordered_map<std::string,std::string> header_;
    std::unordered_map<std::string,std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string,int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch); //16进制转换为10进制

};

#endif //TINYWEBSERVER_HTTPREQUEST_H