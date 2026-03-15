cmake_minimum_required(VERSION 4.0)
project(TinyWebServer)

include_directories(C:\\Program Files\\MySQL\\MySQL Server 8.0\\include)
link_directories(C:\\Program Files\\MySQL\\MySQL Server 8.0\\lib)

link_libraries(libmysql)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyWebServer main.cpp
        src/buffer/buffer.h
        src/buffer/buffer.cpp
        src/Log/Log.h
        src/Log/Log.cpp
        Pool/ThreadPool.h
        Pool/sqlconnpool.h
)

target_link_libraries(TinyWebServer libmysql)
