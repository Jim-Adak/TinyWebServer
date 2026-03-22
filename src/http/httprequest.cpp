//
// Created by Confuse on 2026/3/22.
//

#include"httprequest.h"


const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index","/register","/login",
    "/welcome","/video","/picture",
};

const std::unordered_map<std::string,int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html",0},{"/login.html",1},
};

bool HttpRequest::IsKeepAlive() const {
    if (header_.count("Connection") == 1 ) {
        return header_.find("Connection")->second == "keep-alive" && version_ =="1.1";
    }
    return false;
}
