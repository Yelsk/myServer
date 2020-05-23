/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-23 22:29:01
 * @FilePath: /myWebServer-master/oldversion/0.3/HttpConnection.h
 */ 

#pragma once

#include <mysql/mysql.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include "SQLPool.h"
#include "Timer.h"
#include "Epoll.h"
#include "MutexLock.h"

const int MAX_BUFF = 4096;
const int KEEP_ALIVE_TIME = 5 * 60 * 1000;
const int TIMER_UPDATE_TIME = 500;

//HTTPCODE
const int INTERNAL_ERROR = -1;
const int NO_REQUESTION = 0;
const int GET_REQUESTION = 1;
const int BAD_REQUESTION = 2;
const int FORBIDDEN_REQUESTION = 3;
const int FILE_REQUESTION = 4;
const int NOT_FOUND = 5;
const int POST_FILE = 6;

//CHECKSTATUS
const int HEAD = 8;
const int REQUETION = 9;
const int CONTENT = 10;
const int FINISH = 11;

class HttpConnection
{
public:
    HttpConnection(int _epfd, int _client_fd, int _events, Epoll *_epoll, std::string _path, MutexLock *_lock, TimerQueue *_timerQueue, SQLPool *_sqlpool, sockaddr_in address);
    ~HttpConnection();
    int epfd;
    int client_fd;
    int events;
    int read_count;
    bool HandleRead();
    bool HandleWrite();//响应发送
    void HandleConn();
    void Reset();
    void doit();//线程接口函数
    void close_coon();//关闭客户端链接
    int getFd() {return client_fd;}
    int getEvents(){return events;}
    void setEvents(int _events) {events = _events;}
    void addTimer(Timer *_timer) {timer = _timer;}
    void seperateTimer();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
private:
    std::string read_buffer;
    std::string path;
    std::string file_name;
    std::string method;
    std::string url; //文件名称
    std::string version;
    std::string requestContent;
    std::string host;
    char request_head_buffer[1024];
    int Httpversion;
    int check_index;
    int status;
    int contentLength; //http长度
    int Analyse();//解析Http请求头的函数
    int JudgeLine(int &check_index);//该请求是否是完整的以行\r\n
    int HeadersParse(std::string &line);
    int RequestionLineParse(std::string &line);
    int ContentParse(std::string &line);
    int HandleRequest();
    bool keep_alive;
    bool error;
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;
    int file_size; //文件大小
    int header_size;
    Timer *timer;
    TimerQueue *timerQueue;
    MutexLock *lock;
    Epoll *epoll;
    MYSQL *mysql;
    SQLPool *sqlpool;
    sockaddr_in m_address;
    void unmap();
    bool bad_respond();//语法错误请求响应填充
    bool forbiden_respond();//资源权限限制请求响应的填充
    bool successful_respond();//解析成功请求响应填充
    bool not_found_request();//资源不存在请求响应填充
};