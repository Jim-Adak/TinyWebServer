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

//解析处理
bool HttpRequest::parse(Buffer &buff) {
    const char* CRLF[] = "\r\n"; //行结束符标志（回车换行）
    if (buff.ReadableBytes() <=0) { //没有可读的字符
        return false;
    }
    //读取数据
    while (buff.ReadableBytes() && state_ != FINISH) {
        //从buff中的读指针开始到读指针结束，这块区域是未读取得数据并去除“\r\n"，返回有效数据的行末指针
        const char* lineEnd = std::search(buff.Peek(),buff.BeginWriteConst(),CRLF,CRLF + 2);
        //转换为string类型
        std::string line(buff.Peek(),lineEnd);
        switch (state_) {
            /*
          有限状态机，从请求行开始，每处理完后会自动转入到下一个状态
      */
            case REQUEST_LINE:
                if (!ParseRequestLine_(line)) {
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                if (buff.ReadableBytes() <= 2) {
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if (lineEnd == buff.BeginWrite()){break;} //读完了
        buff.RetrieveUntil(lineEnd + 2); //跳过回车换行
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

//解析路径
void HttpRequest::ParsePath_() {
    if (path_ == "/") {
        path_ = "/index.html";
    }else {
        for (auto &item: DEFAULT_HTML) {
            if (item == path_) {
                path_+=".html";
                break;
            }
        }
    }
}
