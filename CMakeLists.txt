cmake_minimum_required(VERSION 4.0)
project(TinyWebServer)

#include_directories("C:\\Program Files\\MySQL\\MySQL Server 8.0\\include")

#link_directories("C:\\Program Files\\MySQL\\MySQL Server 8.0\\lib")

include_directories(${MYSQL_INCLUDE_DIR})

link_libraries(libmysql)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyWebServer main.cpp
        src/buffer/buffer.h
        src/buffer/buffer.cpp
        src/Log/Log.h
        src/Log/Log.cpp
        src/Pool/ThreadPool.h
        src/Pool/sqlconnpool.h
        src/Pool/sqlconnpool.cpp
        src/Pool/test.cpp
        src/http/httprequest.h
        src/http/httprequest.cpp
        src/http/httpresponse.h
        src/heaptimer/heaptimer.h
        src/heaptimer/heaptimer.cpp
        src/http/httpconn.cpp
        src/epoll/epoller.h
)

#target_link_libraries(TinyWebServer libmysql  ws2_32) #ws2_32是Windows socket库，mysql库需要，在Linux下将ws2_32注释掉
target_link_libraries(TinyWebServer
        ${MYSQL_LIBRARIES}
)
