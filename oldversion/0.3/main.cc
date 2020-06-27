/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-27 16:15:30
 * @FilePath: /myWebServer-master/oldversion/0.3/main.cc
 */ 

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/epoll.h>
#include "ThreadPool.h"
#include "HttpConnection.h"
#include "Log.h"

using namespace std;

const int PORT = 8888;
const int LISTENQ = 5;
const string PATH = "./WEB/";
const int EPOLLWAIT_TIME = 500;
const int TIMER_TIME_OUT = 500;
extern const int MAXEPOLL;


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
socket_bind_listen(const int port, sockaddr_in &address)
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

void
acceptConnection(int listen_fd, Epoll *epoll, MutexLock *lock, TimerQueue *timerQueue, SQLPool *sqlpool)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while (true)
    {
        int accept_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if(accept_fd < 0)
        {
            LOG_ERROR("%s:errno is : %d", "accept error", errno);
            break;
        }
        // 文件描述符可以读，边缘触发(Edge Triggered)模式，保证一个socket连接在任一时刻只被一个线程处理
        uint32_t events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET | EPOLLERR;
        HttpConnection *conn = new HttpConnection(epoll->getEpollfd(), accept_fd, events, epoll, PATH, lock, timerQueue, sqlpool, client_addr);
        //将连接加入epoll事件表
        epoll->epoll_add(conn);
        //设为非阻塞模式
        int ret = setnonblocking(accept_fd);
        if(ret < 0)
        {
            LOG_ERROR("%s", "set nonblocking error");
            return;
        }
        Timer *timer = new Timer(conn, TIMER_TIME_OUT);
        conn->addTimer(timer);
        lock->lock();
        timerQueue->addTimer(timer);
        lock->unlock();
    }
    return;
}

// 分发处理函数
void handle_events(MutexLock *lock, 
                                            Epoll *epoll, 
                                            int listen_fd, 
                                            epoll_event* events, 
                                            int events_num, 
                                            TimerQueue *timerQueue, 
                                            SQLPool *sqlpool,
                                            ThreadPool<HttpConnection> *pool)
{
    for(int i = 0; i < events_num; i++)
    {
        // 获取有事件产生的描述符
        HttpConnection *conn = (HttpConnection *)(events[i].data.ptr);
        int fd = conn->getFd();
        // 有事件发生的描述符为监听描述符
        if(fd == listen_fd)
        {
            acceptConnection(listen_fd, epoll, lock, timerQueue, sqlpool);
        }
        else
        {
            // 排除错误事件
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP))
            {
                delete conn;
                continue;
            }
            else if(events[i].events & EPOLLIN)
            {
                 conn->seperateTimer();
                if(conn->HandleRead())
                {
                    LOG_INFO("deal with the read event of client(%s)", inet_ntoa(conn->get_address()->sin_addr));
                    Log::get_instance()->flush();
                    // 将请求任务加入到线程池中
                    pool->addjob(conn);
                }
                else
                {
                    delete conn;
                }
            }
            else if(events[i].events & EPOLLOUT)
            {
                LOG_INFO("deal with the write event of client(%s)", inet_ntoa(conn->get_address()->sin_addr));
                Log::get_instance()->flush();
                conn->seperateTimer();
                if(conn->HandleWrite())
                {
                    conn->HandleConn();
                }
                else
                {
                    delete conn;
                }
            }
        }
    }
}

void handle_expired_event(MutexLock *lock, TimerQueue *timerQueue)
{
    lock->lock();
    while (!timerQueue->empty())
    {
        Timer *ptimer_now = timerQueue->top();
        if (ptimer_now->isDeleted())
        {
            timerQueue->pop();
            delete ptimer_now;
        }
        else if (ptimer_now->isvalid() == false)
        {
            timerQueue->pop();
            delete ptimer_now;
        }
        else
        {
            break;
        }
    }
    lock->unlock();
}

int main(int argc, char *argv[])
{
    Log::get_instance()->init("ServerLog", 2000, 800000, 100);
    ThreadPool<HttpConnection> *pool = new ThreadPool<HttpConnection>();
    MutexLock *lock = new MutexLock();
    Epoll *epoll = new Epoll();
    TimerQueue *timerQueue = new TimerQueue();
    epoll_event events[10000];
    SQLPool *sqlpool = SQLPool::get_instance();
    sqlpool->init("localhost", "gan", "123", "gandb", 3306, 8);
    LOG_INFO("connect pool success");
    Log::get_instance()->flush();
    // handle_for_sigpipe();
    struct sockaddr_in address;
    int listen_fd = socket_bind_listen(PORT, address);
    if(listen_fd < 0)
    {
        LOG_ERROR("%s", "socket bind failed");
        delete pool;
        delete lock;
        delete timerQueue;
        delete epoll;
        return 1;
    }
    if(setnonblocking(listen_fd) < 0)
    {
        LOG_ERROR("%s", "set nonblocking error");
        delete pool;
        delete lock;
        delete timerQueue;
        delete epoll;
        return 1;
    }
    uint32_t event = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR;
    HttpConnection *conn = new HttpConnection(epoll->getEpollfd(), listen_fd, event, epoll, PATH, lock, timerQueue, sqlpool, address);
    assert(conn);
    epoll->epoll_add(conn);
    while(true)
    {
        int events_num = epoll->my_epoll_wait(events, MAXEPOLL, -1);
        if(events_num < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }
        if(events_num == 0) continue;
        handle_events(lock, epoll, listen_fd, events, events_num, timerQueue, sqlpool, pool);
        handle_expired_event(lock, timerQueue);
    }
    close(listen_fd);
    delete conn;
    delete pool;
    delete lock;
    delete timerQueue;
    delete epoll;
    return 0;
}