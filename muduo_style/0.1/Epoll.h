/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-29 20:55:10
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
    int epoll_add(std::shared_ptr<Channel> request, int timeout);
    int epoll_mod(std::shared_ptr<Channel> request, int timeout);
    int epoll_del(std::shared_ptr<Channel> request);
    std::vector<std::shared_ptr<Channel>> poll();
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
    void add_timer(std::shared_ptr<Channel> request, int timeout);
    int getEpollfd() {return epfd;}
    void handleExpired() { timerQueue.handleExpired(); }

private:
    static const int MAXFDS = 1000;
    int epfd;
    TimerQueue timerQueue;
    std::vector<epoll_event> m_events;
    std::shared_ptr<Channel> m_channels[MAXFDS];
    std::shared_ptr<HttpConnection> m_conns[MAXFDS];
};