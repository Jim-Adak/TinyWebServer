//
// Created by Adak Jim on 2026/4/15.
//

#ifndef TINYWEBSERVER_HTTPRESPONSE_H
#define TINYWEBSERVER_HTTPRESPONSE_H
#include<unordered_map>
#include<fcntl.h> // open
#include<unistd.h> // close
#include<sys/stat.h> // stat
#include<sys/mman.h> // mmap, munmap

#include"../buffer/buffer.h"
#include"../Log/Log.h"


class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();
private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFIleType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char* mmFIle_;
    struct stat mmFIleStat_;

    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE; //后缀类型集
    static const std::unordered_map<int,std::string>CODE_STATUS; //编码状态集
    static const std::unordered_map<int,std::string>CODE_PATH; //编码路径集

};

#endif //TINYWEBSERVER_HTTPRESPONSE_H