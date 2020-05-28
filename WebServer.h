/*
 * @Author: GanShuang
 * @Date: 2020-05-27 10:57:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 19:31:28
 * @FilePath: /myWebServer-master/WebServer.h
 */ 

#pragma once

#include <memory>
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "SQLPool.h"

const int MAXFDS = 1000;

class WebServer
{
public:
    WebServer(EventLoop *loop_, SQLPool *sqlpool_, std::string path_, int threadNum_, int port_);
    ~WebServer() {}
    EventLoop *getLoop() { return m_loop; }
    void start();
    void handleNewConn();
    void handleThisCon() { m_loop->updatePoller(m_acceptchannel); }

private:
    EventLoop *m_loop;
    EventLoopThreadPool *m_threadpool;
    Channel *m_acceptchannel;
    SQLPool *m_sqlpool;
    std::string m_path;
    int m_threadNum;
    bool m_started;
    int m_port;
    int m_listenfd;
};