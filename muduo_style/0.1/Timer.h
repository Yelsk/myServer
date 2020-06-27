/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-29 21:28:33
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
    bool operator()(const std::shared_ptr<Timer> a, const std::shared_ptr<Timer> b) const;
};

class TimerQueue
{
private:
    std::priority_queue<std::shared_ptr<Timer>, std::deque<std::shared_ptr<Timer>>, TimerCmp> m_queue;
public:
    TimerQueue();
    ~TimerQueue();
    void addTimer(std::shared_ptr<HttpConnection> conn_, int timeout);
    void handleExpired();
};