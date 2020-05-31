/*
 * @Author: GanShuang
 * @Date: 2020-05-26 10:40:36
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 11:37:22
 * @FilePath: /myWebServer-master/Util.cc
 */ 

#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "Util.h"

const int LISTENQ = 5;

 //设置非阻塞I/O
int
setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//处理SIGPIPE信号，忽略
void
handle_for_sigpipe()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}

int
socket_bind_listen(const int port)
{
    //检查port是否合法
    if(port < 1024 || port > 65535) return -1;

    //创建socket(IPv4 + TCP)，返回监听fd
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
    //消除bind时"Address already"
    // int optval = 1;
    // if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) return -1;
 
    //设置监听IP和PORT，并与监听fd绑定
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htons(INADDR_ANY);
    address.sin_port = htons(port);
    if(bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) == -1) return -1;

    //开始监听，最大等待队列列长为LISTENQ
    if(listen(listen_fd, LISTENQ) == -1) return -1;

    //监听fd无效
    if(listen_fd == -1){
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

void shutdownwr(int fd) {
    shutdown(fd, SHUT_WR);
}

void setsocketnodelay(int fd) {
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}