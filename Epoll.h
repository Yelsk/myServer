/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-28 16:55:15
 * @FilePath: /myWebServer-master/Epoll.h
 */ 

#pragma once

#include <sys/epoll.h>
#include "Timer.h"

class HttpConnection;
class Channel;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_add(Channel *request, int timeout);
    int epoll_mod(Channel *request, int timeout);
    int epoll_del(Channel *request);
    std::vector<Channel *> poll();
    std::vector<Channel *> getEventsRequest(int events_num);
    void add_timer(Channel *request, int timeout);
    int getEpollfd() {return epfd;}
    void handleExpired() { timerQueue.handleExpired(); }

private:
    static const int MAXFDS = 1000;
    int epfd;
    TimerQueue timerQueue;
    std::vector<epoll_event> m_events;
    std::vector<Channel *> m_channels;
    std::shared_ptr<HttpConnection> m_conns[MAXFDS];
};

extern const int MAXEPOLL;