/*
 * @Author: GanShuang
 * @Date: 2020-05-25 22:29:16
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 09:53:50
 * @FilePath: /myWebServer-master/Channel.h
 */ 

#pragma once

#include <sys/epoll.h>
#include <functional>
#include "NonCopyable.h"

class EventLoop;
class HttpConnection;

class Channel : NonCopyable
{
private:
    typedef std::function<void()> CallBack;
    EventLoop *m_loop;
    HttpConnection *m_conn;
    int m_fd;
    uint32_t m_events;

public:
    Channel(EventLoop *loop_);
    Channel(EventLoop *loop_, int fd);
    ~Channel();
    int getFd() { return m_fd; }
    void setFd(int fd) { m_fd = fd; }
    void setConn(HttpConnection *conn) {m_conn = conn; }
    HttpConnection *getConn() { return m_conn; }

    void setReadHandler(CallBack &&readHandler_) { m_readHandler = readHandler_; }
    void setWriteHandler(CallBack &&writeHandler_) { m_writeHandler = writeHandler_; }
    void setErrorHandler(CallBack &&errorHandler_) { m_errorHandler = errorHandler_; }
    void setConnHandler(CallBack &&connHandler_) { m_connHandler = connHandler_; }

    void handleRead();
    void handleWrite();
    void handleError(int fd, int err_num, std::string msg);
    void handleConn();

    void handleEvents() {
        if((m_events & EPOLLHUP) && !(m_events & EPOLLIN)){
            m_events = 0;
            return;
        }
        if(m_events & EPOLLERR){
            if(m_errorHandler) m_errorHandler();
            m_events = 0;
            return;
        }
        if(m_events & (EPOLLIN | EPOLLPRI | EPOLLHUP)){
            handleRead();
        }
        if(m_events & EPOLLOUT){
            handleWrite();
        }
        handleConn();
    }

    void setEvents(uint32_t ev) { m_events = ev; }
    uint32_t getEvents() {return m_events; }

private:
    CallBack m_readHandler;
    CallBack m_writeHandler;
    CallBack m_errorHandler;
    CallBack m_connHandler;

};
