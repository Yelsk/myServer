/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 20:07:09 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 20:38:27
 */

#pragma once

#include <sys/time.h>
#include <queue>

class HttpConnection;

class Timer
{
private:
    bool deleted;
    size_t expired_time;
    HttpConnection *conn;
public:
    Timer(HttpConnection *_conn, int timeout);
    ~Timer();
    void update(int timeout);
    bool isvalid();
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
    std::priority_queue<Timer *, std::deque<Timer *>, TimerCmp> timerQueue;
public:
    TimerQueue();
    ~TimerQueue();
    void addTimer(Timer *timer);
    void addTimer(HttpConnection *_conn, int timeout);
    void handleExpired();
    bool empty() {return timerQueue.empty();}
    Timer *top() {return timerQueue.top();}
    void pop() {timerQueue.pop(); return;}
};