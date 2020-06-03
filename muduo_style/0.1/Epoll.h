/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-03 10:56:31
 * @FilePath: /myWebServer-master/muduo_style/0.1/Epoll.h
 */ 

#pragma once

#include <sys/epoll.h>
#include "Timer.h"

using std::shared_ptr;
using std::vector;

class HttpConnection;
class Channel;

class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_add(shared_ptr<Channel> request, int timeout);
    int epoll_mod(shared_ptr<Channel> request, int timeout);
    int epoll_del(shared_ptr<Channel> request);
    vector<shared_ptr<Channel>> poll();
    vector<shared_ptr<Channel>> getEventsRequest(int events_num);
    void add_timer(shared_ptr<Channel> request, int timeout);
    int getEpollfd() {return epfd;}
    void handleExpired() { timerQueue.handleExpired(); }

private:
    static const int MAXFDS = 1000;
    int epfd;
    TimerQueue timerQueue;
    vector<epoll_event> m_events;
    shared_ptr<Channel> m_channels[MAXFDS];
    shared_ptr<HttpConnection> m_conns[MAXFDS];
};