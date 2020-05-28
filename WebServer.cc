/*
 * @Author: GanShuang
 * @Date: 2020-05-27 10:57:50
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-28 17:14:36
 * @FilePath: /myWebServer-master/WebServer.cc
 */ 

#include "WebServer.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "HttpConnection.h"
#include "Util.h"
#include "Log.h"

WebServer::WebServer(EventLoop *loop_,
    SQLPool *sqlpool_,
    string path_,
    int threadNum_,
    int port_)
    : m_loop(loop_),
    m_sqlpool(sqlpool_),
    m_path(path_),
    m_threadNum(threadNum_),
    m_port(port_),
    m_threadpool(new EventLoopThreadPool(loop_, threadNum_)),
    m_acceptchannel(new Channel(loop_)),
    m_started(false),
    m_listenfd(socket_bind_listen(port_))
{
    m_acceptchannel->setFd(m_listenfd);
    handle_for_sigpipe();
    if(setnonblocking(m_listenfd) < 0){
        LOG_ERROR("setnonblocking failed!!");
        abort();
    }
}

void
WebServer::start()
{
    m_threadpool->start();
    m_acceptchannel->setEvents(EPOLLIN | EPOLLET);
    m_acceptchannel->setReadHandler(bind(&WebServer::handleNewConn, this));
    m_acceptchannel->setConnHandler(bind(&WebServer::handleThisCon, this));
    m_loop->addToPoller(m_acceptchannel, 0);
    m_started = true;
}

void
WebServer::handleNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while (true)
    {
        int accept_fd = accept(m_listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        EventLoop *loop = m_threadpool->getNextLoop();
        if(accept_fd < 0)
        {
            if(errno == EAGAIN)
            {
                break;
            }
            LOG_ERROR("%s:errno is : %d", "accept error", errno);
            break;
        }
        if (accept_fd >= MAXFDS) {
            close(accept_fd);
            continue;
        }
        int ret = setnonblocking(accept_fd);
        if(ret < 0)
        {
            LOG_ERROR("%s", "set nonblocking error");
            return;
        }
        setsocketnodelay(accept_fd);
        // 文件描述符可以读，边缘触发(Edge Triggered)模式，保证一个socket连接在任一时刻只被一个线程处理
        shared_ptr<HttpConnection> conn(new HttpConnection(accept_fd, loop, m_path, m_sqlpool, client_addr));
        conn->getChannel()->setConn(conn);
        loop->queueInLoop(std::bind(&HttpConnection::newEvent, conn));
    }
    m_acceptchannel->setEvents(EPOLLIN | EPOLLET);
}