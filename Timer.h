/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 18:57:11
 * @FilePath: /myWebServer-master/Timer.h
 */ 

#pragma once

#include <memory>
#include <sys/time.h>
#include <queue>

class HttpConnection;

class Timer
{
private:
    bool m_deleted;
    size_t expired_time;
    std::shared_ptr<HttpConnection> m_conn;
public:
    Timer(std::shared_ptr<HttpConnection> conn_, int timeout);
    ~Timer();
    void update(int timeout);
    bool isValid();
    
    void clearConn();
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;
};

class TimerCmp
{
public:
    TimerCmp(){};
    ~TimerCmp(){};
    bool operator()(const Timer *a, const Timer *b) const;
};

class TimerQueue
{
private:
    std::priority_queue<Timer *, std::deque<Timer *>, TimerCmp> m_queue;
public:
    TimerQueue();
    ~TimerQueue();
    void addTimer(Timer *timer);
    void addTimer(std::shared_ptr<HttpConnection> conn_, int timeout);
    void handleExpired();
};