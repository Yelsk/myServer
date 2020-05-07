/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 21:17:55 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-06 09:34:20
 */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

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

#define READ_BUF 2000

class HttpConnection
{
public:
    enum HTTP_CODE{NO_REQUESTION,  //代表请求不完整，需要客户继续输入
    GET_REQUESTION,  //是HTTP请求语法不正确
    BAD_REQUESTION,  //代表获得并且解析了一个正确的HTTP请求
    FORBIDDEN_REQUESTION,  //是代表访问资源的权限有问题
    FILE_REQUESTION,  //代表GET方法资源请求
    INTERNAL_ERROR,  //代表服务器自身问题
    NOT_FOUND,  //代表请求的资源文件不存在
    DYNAMIC_FILE,  //表示是一个动态请求
    POST_FILE}; //表示获得一个以POST方式请求的HTTP请求

    enum CHECK_STATUS{HEAD, //请求解析的状态转移, HEAD表示解析头部信息
    REQUESTION}; //REQUESTION表示解析请求行

    HttpConnection(){      };
    ~HttpConnection(){       };
    int epfd;
    int client_fd;
    int read_count;
    void init(int e_fd, int c_fd);//初始化
    int myread();//读取请求
    bool mywrite();//响应发送
    void doit();//线程接口函数
    void close_coon();//关闭客户端链接
private:
    char request_head_buf[1000]; //响应头的填充
    char post_buf[1000]; //Post请求的读缓冲区
    char read_buf[READ_BUF]; //客户端的http请求读取
    char filename[250]; //文件总目录
    int file_size; //文件大小
    int check_index; //目前检测到的位置
    int read_buf_len; //读取缓冲区大小
    char *method; //请求方法
    char *url; //文件名称
    char *version; //协议版本
    char *argv; //动态请求参数
    bool m_linger; //是否保持连接
    int m_http_count; //http长度
    char *m_host; //主机名记录
    char path_400[17]; //出错码400打开的文件名缓冲区
    char path_403[23]; //出错码403打开返回的文件名缓冲区
    char path_404[40]; //出错码404对应文件名缓冲区
    char message[1000]; //响应消息体缓冲区
    char body[2000]; //post响应消息体缓冲区
    CHECK_STATUS status; //状态转移
    bool m_flag; //true表示是动态请求，反之是静态请求
    HTTP_CODE analyse();//解析Http请求头的函数
    int judge_line(int &check_index, int &read_buf_len);//该请求是否是完整的以行\r\n
    HTTP_CODE head_analyse(char *temp);//http请求头解析
    HTTP_CODE requestion_analyse(char *temp);//http请求行解析
    HTTP_CODE do_post();//对post请求中的参数进行解析
    HTTP_CODE do_file();//对GET请求方法中的url 协议版本的分离
    void modfd(int epfd, int sock, int ev);//改变socket为状态
    void dynamic(char *filename, char *argv);//通过get方法进入的动态请求处理
    void post_respond();//POST请求响应填充
    bool bad_respond();//语法错误请求响应填充
    bool forbiden_respond();//资源权限限制请求响应的填充
    bool successful_respond();//解析成功请求响应填充
    bool not_found_request();//资源不存在请求响应填充
};

#endif