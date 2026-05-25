# TinyWebServer

一个用 C++ 实现的轻量级 Linux HTTP 服务器。基于 **epoll (ET/LT) + 线程池 + 非阻塞 I/O** 的 Reactor 并发模型，配合 **MySQL 连接池** 完成用户的注册与登录，支持静态资源访问、文件上传、Keep-Alive 长连接，以及一个生产级的 **异步日志系统**。

> 本项目是参考 [JehanRio/TinyWebServer](https://github.com/JehanRio/TinyWebServer) 一行一行手敲实现的学习项目，用于练习 Linux C++ 服务端编程、I/O 多路复用、数据库连接池、RAII 资源管理等核心知识。

---

## 目录

- [功能特性](#功能特性)
- [整体架构](#整体架构)
- [项目目录结构](#项目目录结构)
- [核心模块说明](#核心模块说明)
- [一次 HTTP 请求的完整流程](#一次-http-请求的完整流程)
- [环境依赖](#环境依赖)
- [构建与运行](#构建与运行)
- [数据库初始化](#数据库初始化)
- [配置说明](#配置说明)
- [接口测试](#接口测试)
- [日志系统说明](#日志系统说明)
- [常见问题 FAQ](#常见问题-faq)

---

## 功能特性

- ✅ 基于 **epoll** 的 I/O 多路复用，支持 **ET（边缘触发）/ LT（水平触发）** 任意组合
- ✅ **主从 Reactor + 线程池**：主线程负责 accept / 事件分发，工作线程负责业务处理（读写 + 解析）
- ✅ **EPOLLONESHOT** 保证同一连接同一时刻只被一个线程处理
- ✅ HTTP/1.1 请求解析（**有限状态机**：REQUEST_LINE → HEADERS → BODY → FINISH）
- ✅ 支持 GET / POST，HTML 静态资源访问（mmap 文件映射 + writev 聚集写）
- ✅ **MySQL 连接池**（信号量 + 互斥锁），用户注册 / 登录
- ✅ **小顶堆定时器** 关闭空闲长连接
- ✅ **单例 + 阻塞队列** 实现的异步日志，可按天 / 行数自动切分日志文件
- ✅ **RAII** 管理数据库连接、文件映射、日志线程等所有资源

---

## 整体架构

```
                  ┌───────────────────────────────────────────┐
                  │              主线程 (main reactor)         │
                  │  epoll_wait → accept → 注册到 epoll        │
                  └───────────────────────────────────────────┘
                                       │
              ┌────────────────────────┼────────────────────────┐
              │                        │                        │
        EPOLLIN 事件              EPOLLOUT 事件             RDHUP/HUP/ERR
              │                        │                        │
              ▼                        ▼                        ▼
      threadpool.AddTask         threadpool.AddTask          CloseConn
       (OnRead → process)         (OnWrite via writev)
              │                        │
              ▼                        ▼
      ┌─────────────────────────────────────────┐
      │           工作线程池 (sub reactor)         │
      │   读 socket → 解析 → 生成响应 → 切换事件   │
      └─────────────────────────────────────────┘
                       │
                       ▼
        ┌────────────────────────────┐
        │   小顶堆定时器 (HeapTimer)   │  ← 每轮 epoll_wait 计算下一次最早过期时间作为 timeout
        │  超时连接自动调用 CloseConn  │
        └────────────────────────────┘

        ┌────────────────────────────┐
        │   MySQL 连接池 (SqlConnPool) │  ← 通过 SqlConnRAII 获取/归还连接
        └────────────────────────────┘

        ┌────────────────────────────┐
        │   异步日志 (Log + 阻塞队列)   │  ← 业务线程仅入队；独立写线程刷盘
        └────────────────────────────┘
```

---

## 项目目录结构

```
TinyWebServer/
├── main.cpp                          # 入口：构造 WebServer 并 Start()
├── CMakeLists.txt
├── resources/                        # 静态资源根目录（HTML 等）
│   ├── index.html
│   ├── login.html
│   ├── register.html
│   ├── welcome.html
│   ├── error.html
│   ├── 400.html / 403.html / 404.html
├── log/                              # 运行时生成，按天命名 YYYY_MM_DD.log
└── src/
    ├── buffer/                       # 自动扩容的读写 Buffer（仿 muduo）
    │   ├── buffer.h
    │   └── buffer.cpp
    ├── Log/                          # 异步日志系统
    │   ├── Log.h / Log.cpp           # 单例日志器
    │   └── blockqueue.h              # 模板阻塞队列
    ├── Pool/
    │   ├── ThreadPool.h              # 基于 shared_ptr 的线程池
    │   ├── sqlconnpool.h             # MySQL 连接池 + RAII 包装
    │   └── sqlconnpool.cpp
    ├── epoll/                        # epoll 封装
    │   ├── epoller.h
    │   └── epoller.cpp
    ├── heaptimer/                    # 小顶堆定时器
    │   ├── heaptimer.h
    │   └── heaptimer.cpp
    ├── http/
    │   ├── httprequest.{h,cpp}       # HTTP 请求解析（状态机）
    │   ├── httpresponse.{h,cpp}      # HTTP 响应生成（mmap）
    │   └── httpconn.{h,cpp}          # 单个客户端连接：read/write/process
    └── webserver/
        ├── webserver.h
        └── webserver.cpp             # 服务器总入口，事件循环
```

---

## 核心模块说明

### 1. `Buffer` — 自动扩容的应用层缓冲区

参考 muduo 的 `Buffer` 实现，内部用 `std::vector<char>` + 读 / 写两个游标 `readPos_` / `writePos_`，支持：

- `ReadFd / WriteFd`：在用户态用 `readv` 一次性读到 buffer 内 + 一个 64KB 栈上扩展区，避免短读
- `Append / Retrieve / EnsureWriteable`：写入、消费、必要时移动或 resize
- 空间不足时优先复用 `readPos_` 之前已读区域，仅在仍不够时才 resize

参见 [src/buffer/buffer.h](src/buffer/buffer.h)。

### 2. `Log` + `BlockQueue` — 异步日志

- 采用 **局部静态变量法** 实现的懒汉单例 `Log::Instance()`
- 业务线程仅把格式化好的字符串 `push_back` 到 `BlockQueue<std::string>`，**不做磁盘 I/O**
- 一个独立写线程 `FlushLogThread` 调用 `AsyncWrite_()`，`pop` 出来 `fputs` 到文件
- 同步模式（`maxQueueCapacity == 0`）下退化为业务线程直接写文件
- 自动按 **日期** 和 **MAX_LINES = 50000** 切分新日志文件
- 提供 4 个宏：`LOG_DEBUG / LOG_INFO / LOG_WARN / LOG_ERROR`，配合 `level_` 过滤

源码：[src/Log/Log.cpp](src/Log/Log.cpp)、[src/Log/blockqueue.h](src/Log/blockqueue.h)

### 3. `ThreadPool` — 线程池

- 构造时 `detach` 出 N 个工作线程，所有线程共享同一个 `Pool`（用 `shared_ptr` 持有，保证线程安全析构）
- 任务用 `std::function<void()>` 存放在 `std::queue` 中
- 取任务前持锁，**取出后立刻 unlock 再执行任务**，避免长时间持锁阻塞其他线程

参见 [src/Pool/ThreadPool.h](src/Pool/ThreadPool.h)。

### 4. `SqlConnPool` + `SqlConnRAII` — 数据库连接池

- 初始化时一次性 `mysql_real_connect` 出 N 个连接放进 `std::queue<MYSQL*>`
- `sem_t` 信号量做"可用连接数"计数，**互斥锁**保护队列
- 使用方通过 RAII 取连接：

  ```cpp
  MYSQL* sql;
  SqlConnRAII raii(&sql, SqlConnPool::Instance()); // 构造时 GetConn
  // ... 使用 sql ...
  // 离开作用域自动 FreeConn
  ```

源码：[src/Pool/sqlconnpool.h](src/Pool/sqlconnpool.h)、[src/Pool/sqlconnpool.cpp](src/Pool/sqlconnpool.cpp)。

### 5. `Epoller` — epoll 的薄封装

- `AddFd / ModFd / DelFd / Wait`
- 内部 `std::vector<struct epoll_event>` 默认容量 1024

参见 [src/epoll/epoller.h](src/epoll/epoller.h)。

### 6. `HeapTimer` — 小顶堆定时器

- 节点 `TimerNode { id, expires, cb }`，按 `expires` 维护小顶堆
- `add / adjust / del_` 都通过 `siftup_ / siftdown_` 调整
- `unordered_map<int, size_t> ref_` 让"按 id 查节点"O(1)
- 主循环每次 `epoll_wait` 之前调 `GetNextTick()`，它会 `tick()` 清掉所有已过期连接、再返回下一次超时还有多久（毫秒），作为 `epoll_wait` 的 timeout

源码：[src/heaptimer/heaptimer.cpp](src/heaptimer/heaptimer.cpp)。

### 7. `HttpRequest` — 请求解析（有限状态机）

```
REQUEST_LINE  →  HEADERS  →  BODY  →  FINISH
```

- `parse(Buffer&)` 每次循环用 `std::search` 找到一行 `\r\n`，按当前状态分派给 `ParseRequestLine_ / ParseHeader_ / ParseBody_`
- 请求行用正则 `^([^ ]*) ([^ ]*) HTTP/([^ ]*)$` 抽取 method / path / version
- POST `application/x-www-form-urlencoded` 在 `ParseFromUrlencoded_` 中按 `=` / `&` / `+` / `%XX` 解析
- 登录 / 注册由 `UserVerify(name, pwd, isLogin)` 走 `SqlConnRAII` 查 / 写 `webserver.user` 表

源码：[src/http/httprequest.cpp](src/http/httprequest.cpp)。

### 8. `HttpResponse` — 响应生成 + mmap 零拷贝文件

- `MakeResponse` 先 `stat` 检查文件是否存在 / 可读，决定 200/403/404
- `AddContent_` 用 **`mmap` 将文件映射到内存**，failure 时返回 `MAP_FAILED`
- 配合 `HttpConn::write` 的 `writev` 一次性写出"响应头 + 文件映射区"两段数据（scatter-gather）
- `UnmapFile / munmap` 在 `Close` / 析构时归还映射

源码：[src/http/httpresponse.cpp](src/http/httpresponse.cpp)。

### 9. `HttpConn` — 单个客户端连接

- 持有读写两个 `Buffer`、`iovec iov_[2]`、`HttpRequest`、`HttpResponse`
- `read` / `write` 在 ET 模式下都用 `do { ... } while(isET)` 循环榨干 socket
- `write` 中对 `writev` 的返回值做了**部分写**处理：分别推进 `iov_[0]`、`iov_[1]` 的指针和剩余长度
- `process()` 是真正的"读完一次请求 → 解析 → 生成响应 → 装配 iov"

源码：[src/http/httpconn.cpp](src/http/httpconn.cpp)。

### 10. `WebServer` — 总控

- 构造时初始化日志、SQL 连接池、Epoller、线程池、定时器、监听 socket
- `Start()` 主循环：
  - `GetNextTick()` 取最近一次超时还要多久 → 作为 `epoll_wait` 的 timeout
  - 遍历返回的事件：
    - `listenFd` → `DealListen_` accept 新连接
    - `EPOLLRDHUP/HUP/ERR` → `CloseConn_`
    - `EPOLLIN` → 线程池 `AddTask(OnRead_)`
    - `EPOLLOUT` → 线程池 `AddTask(OnWrite_)`
- `OnProcess` 在 `process()` 成功后把 fd 改成监听写事件，发送完后再切回读

源码：[src/webserver/webserver.cpp](src/webserver/webserver.cpp)。

---

## 一次 HTTP 请求的完整流程

以 `POST /login.html` 为例：

1. **accept**：主线程 `epoll_wait` 在 listenFd 上拿到 EPOLLIN，`DealListen_` 循环 accept 出新 connFd，注册到 epoll（`EPOLLIN | EPOLLONESHOT | EPOLLET`），并 `timer_->add(fd, 60000ms, CloseConn)`。
2. **read 触发**：客户端发送数据，主线程在 connFd 上拿到 `EPOLLIN`，通过 `threadpool_->AddTask(OnRead_)` 把读任务丢给工作线程。
3. **工作线程读 + 解析**：
   - `HttpConn::read` 在 ET 下循环 `recv` 到 `readBuff_`
   - `HttpConn::process` → `HttpRequest::parse` 状态机解析出 method=POST、path=/login.html、Content-Type=application/x-www-form-urlencoded 等
   - `ParsePost_` 命中 `DEFAULT_HTML_TAG`，从 `SqlConnPool` 拿连接执行 `SELECT username,password FROM user WHERE ...`
   - 验证通过 → `path_ = /welcome.html`；失败 → `path_ = /error.html`
   - `HttpResponse::MakeResponse` 写入响应头到 `writeBuff_`，并 `mmap` 出对应 HTML
   - 装配 `iov_[0]=writeBuff_`、`iov_[1]=mmap区域`
4. **切换为写**：`OnProcess` 把 fd 改成 `EPOLLOUT | EPOLLONESHOT | EPOLLET`，主线程下一轮 `epoll_wait` 就会拿到。
5. **写**：主线程 `AddTask(OnWrite_)` → `writev` 一次性写出响应头 + 文件，对部分写的情况推进 iov 指针。
6. **Keep-Alive / 关闭**：写完后若是长连接 → fd 改回 `EPOLLIN`；否则 `CloseConn_` 关闭。
7. **定时器**：每次发生读 / 写事件都会 `ExtentTime_(client)` 推迟超时；如果客户端一直不动，到点后 `HeapTimer::tick` 会自动调用注册的 `CloseConn_` 回调。

---

## 环境依赖

| 项目 | 版本 |
|---|---|
| 操作系统 | Linux (推荐 Ubuntu 22.04+ / WSL2 Ubuntu) |
| 编译器 | g++ ≥ 13（需支持 C++20） |
| 构建工具 | CMake ≥ 3.20（项目声明 `cmake_minimum_required(VERSION 4.0)`，按需下调） |
| 数据库 | MySQL ≥ 5.7 / 8.0 |
| 其他库 | `libmysqlclient-dev`、`pthread` |

```bash
sudo apt update
sudo apt install -y build-essential cmake libmysqlclient-dev mysql-server
```

> ⚠️ 本项目使用了 Linux 专有接口（epoll、`sys/uio.h`、`sys/mman.h`、`sys/epoll.h`、`arpa/inet.h` 等），**不能在原生 Windows 下编译运行**，请使用 Linux 或 WSL2。

---

## 构建与运行

```bash
# 1. 进入项目根目录
cd TinyWebServer

# 2. 配置 & 编译
cmake -S . -B build
cmake --build build -j

# 3. 准备日志目录（首次运行需要，否则 init() 时会自动 create_directory）
mkdir -p log

# 4. 启动
./build/TinyWebServer
```

启动成功后，日志会输出类似：

```
========== Server init ==========
Port:1316, OpenLinger: false
Listen Mode: ET, OpenConn Mode: ET
LogSys level: 1
srcDir: /your/path/TinyWebServer/resources/
SqlConnPool num: 12, ThreadPool num: 6
Server port:1316
======== Server start =========
```

浏览器访问 `http://<服务器IP>:1316/` 即可看到首页。

---

## 数据库初始化

`main.cpp` 中默认连接的是 `webserver` 库，用户表名 `user`。请先在 MySQL 里建好：

```sql
CREATE DATABASE IF NOT EXISTS webserver CHARACTER SET utf8mb4;
USE webserver;

CREATE TABLE IF NOT EXISTS user (
    username VARCHAR(50) NOT NULL PRIMARY KEY,
    password VARCHAR(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 可选：插入一个测试账号
INSERT INTO user(username, password) VALUES('admin', '123456');
```

然后把 [main.cpp](main.cpp) 里的 MySQL 账号密码改成你自己的：

```cpp
WebServer server(
    1316, 3, 60000, false,
    3306, "root", "123456", "webserver",   // ← sqlPort, sqlUser, sqlPwd, dbName
    12, 6, true, 1, 1024);
```

---

## 配置说明

[main.cpp](main.cpp) 里 `WebServer` 构造函数的参数列表：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `port` | 1316 | 监听端口 |
| `trigMode` | 3 | 0=listen LT/conn LT；1=listen LT/conn ET；2=listen ET/conn LT；3=listen ET/conn ET |
| `timeoutMS` | 60000 | 单个连接超时时间（毫秒），0 表示不启用定时器 |
| `OptLinger` | false | 是否打开 SO_LINGER 优雅关闭（当前未在 InitSocket_ 中启用，预留参数） |
| `sqlPort` | 3306 | MySQL 端口 |
| `sqlUser` | "root" | MySQL 用户名 |
| `sqlPwd` | "123456" | MySQL 密码 |
| `dbName` | "webserver" | 数据库名 |
| `connPoolNum` | 12 | MySQL 连接池连接数 |
| `threadNum` | 6 | 工作线程数 |
| `openLog` | true | 是否开启日志 |
| `logLevel` | 1 | 日志级别：0=DEBUG、1=INFO、2=WARN、3=ERROR |
| `logQueSize` | 1024 | 异步日志阻塞队列容量，0 表示同步日志 |

---

## 接口测试

启动服务后用 `curl` 验证：

```bash
# 1. 首页（自动跳转 index.html）
curl -i http://127.0.0.1:1316/

# 2. 登录页
curl -i http://127.0.0.1:1316/login.html

# 3. 注册页
curl -i http://127.0.0.1:1316/register.html

# 4. 404
curl -i http://127.0.0.1:1316/not-exist.html

# 5. 注册新用户（form-urlencoded）
curl -i -X POST -d 'username=alice&password=pw1234' \
     http://127.0.0.1:1316/register.html
# → 返回 welcome.html

# 6. 登录正确密码
curl -i -X POST -d 'username=alice&password=pw1234' \
     http://127.0.0.1:1316/login.html
# → 返回 welcome.html

# 7. 登录错误密码
curl -i -X POST -d 'username=alice&password=WRONG' \
     http://127.0.0.1:1316/login.html
# → 返回 error.html
```

---

## 日志系统说明

- 日志文件位于 `./log/`，按当天日期命名：`2026_05_25.log`
- 一旦超过 `MAX_LINES = 50000` 条会自动切分：`2026_05_25-1.log`、`2026_05_25-2.log`...
- 跨天会切换到新文件
- 同一行格式：

  ```
  2026-05-25 15:09:43.522264 [info] : Client[20](127.0.0.1:59059) in, userCount:1
  ```

- 优雅关停（SIGTERM / SIGINT）时，析构函数会：
  1. 唤醒并等阻塞队列里残留的日志全部消费完
  2. `Close()` 队列、`join` 写线程
  3. `flush()` + `fclose(fp_)`，保证日志不丢

---

## 常见问题 FAQ

**Q1：编译报错找不到 `mysql/mysql.h`？**
A：没装 `libmysqlclient-dev`。`sudo apt install libmysqlclient-dev`。

**Q2：能在 Windows 上跑吗？**
A：不能直接跑。代码用了 epoll/mmap/uio 等 Linux 专有接口。推荐用 **WSL2 Ubuntu** 编译运行。

**Q3：日志目录不存在会怎样？**
A：`Log::init()` 会在 `fopen` 失败时调用 `std::filesystem::create_directory(path_)` 自动创建。

**Q4：日志文件一直是空的？**
A：检查 `Log::IsOpen()` 是否为 true（由 `init()` 设置）。如果在 `WebServer` 构造完成前就调用了 `LOG_xxx`，日志会被丢弃（这是宏 `LOG_BASE` 的设计）。

**Q5：怎么换端口？**
A：改 [main.cpp:7](main.cpp#L7) 第一个参数。

**Q6：怎么调成同步日志？**
A：把 [main.cpp:9](main.cpp#L9) 最后一个参数 `1024` 改成 `0`。

**Q7：为什么用 ET 模式？**
A：ET 触发次数少 → 减少 `epoll_wait` 返回次数 → 高并发下性能更好。代价是必须在一次事件中"读到 EAGAIN 为止"，否则会丢事件，所以 `HttpConn::read` 是 `do { ... } while (isET);`。

---

## 参考

- 陈硕《Linux 多线程服务端编程》——`Buffer` / Reactor 思想
- 游双《Linux 高性能服务器编程》——epoll、状态机解析

