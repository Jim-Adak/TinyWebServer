cmake_minimum_required(VERSION 4.0)
project(TinyWebServer)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyWebServer main.cpp
        src/buffer/buffer.h
        src/buffer/buffer.cpp
        src/Log/Log.h
        src/Log/Log.cpp
        Pool/ThreadPool.h
        Pool/sqlconnpool.h
)
