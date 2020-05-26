/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 10:20:41
 * @FilePath: /myWebServer-master/Timer.h
 */ 

#pragma once

#include <sys/time.h>
#include <queue>

class HttpConnection;

class Timer
{
private:
    bool m_deleted;
    size_t expired_time;
    HttpConnection *m_conn;
public:
    Timer(HttpConnection *conn_, int timeout);
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
    void addTimer(HttpConnection *_conn, int timeout);
    void handleExpired();
};