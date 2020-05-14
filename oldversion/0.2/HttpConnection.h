/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 21:17:55 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-07 17:06:36
 */

#pragma once

#include<iostream>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/sendfile.h>
#include<sys/epoll.h>
#include<sys/fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include "Timer.h"
#include "Epoll.h"

const int MAX_BUFF = 4096;

//HTTPCODE
const int INTERNAL_ERROR = -1;
const int NO_REQUESTION = 0;
const int GET_REQUESTION = 1;
const int BAD_REQUESTION = 2;
const int FORBIDDEN_REQUESTION = 3;
const int FILE_REQUESTION = 4;
const int NOT_FOUND = 5;
const int DYNAMIC_FILE = 6;
const int POST_FILE = 7;

//CHECKSTATUS
const int HEAD = 8;
const int REQUETION = 9;
const int FINISH = 10;

class HttpConnection
{
public:
    HttpConnection(int _epfd, int _client_fd, int _events, Epoll *_epoll, std::string _path);
    ~HttpConnection();
    int epfd;
    int client_fd;
    int events;
    int read_count;
    size_t myread();//读取请求
    bool HandleRead();
    bool HandleWrite();//响应发送
    void doit();//线程接口函数
    void close_coon();//关闭客户端链接
    int getFd() {return client_fd;}
    int getEvents(){return events;}
    void setEvents(int _events) {events = _events;}
    void addTimer(Timer *_timer) {timer = _timer;}
    void seperateTimer();
private:
    std::string read_buffer;
    std::string request_head_buffer;
    std::string post_buffer;
    std::string path;
    std::string file_name;
    std::string argv;
    std::string method;
    std::string url; //文件名称
    std::string version;
    std::string contentBody;
    std::string host;
    int Httpversion;
    int check_index;
    int status;
    int contentLength; //http长度
    int Analyse();//解析Http请求头的函数
    int JudgeLine(int &check_index);//该请求是否是完整的以行\r\n
    int HeadersParse(std::string &line);
    int RequestionLineParse(std::string &line);
    int HandleGet();
    int HandlePost();
    bool dynamic_flag;
    bool keep_alive;
    bool error;
    // char request_head_buf[1000]; //响应头的填充
    // char post_buf[1000]; //Post请求的读缓冲区
    // char read_buf[READ_BUF]; //客户端的http请求读取
    // char filename[250]; //文件总目录
    int file_size; //文件大小
    Timer *timer;
    Epoll *epoll;
    void dynamic(std::string &filename, std::string &argv);//通过get方法进入的动态请求处理
    void post_respond();//POST请求响应填充
    bool bad_respond();//语法错误请求响应填充
    bool forbiden_respond();//资源权限限制请求响应的填充
    bool successful_respond();//解析成功请求响应填充
    bool not_found_request();//资源不存在请求响应填充
};