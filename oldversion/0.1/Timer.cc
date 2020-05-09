/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 20:19:04 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 20:38:16
 */

#include "Timer.h"

using namespace std;

Timer::Timer(HttpConnection *_conn, int timeout)
                    : deleted(false), 
                    conn(_conn)
{
    //cout << "myTimer()" << endl;
    struct timeval now;
    gettimeofday(&now, nullptr);
    // 以毫秒计
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

Timer::~Timer()
{
    //cout << "~myTimer" << endl;
    if(conn)
    {
        cout << "connection = " << conn << endl;
        delete conn;
        conn = nullptr;
    }
}

void
Timer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool
Timer::isvalid()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    size_t tmp = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    if(tmp < expired_time)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

void
Timer::clearConn()
{
    conn = nullptr;
    this->setDeleted();
}

bool
Timer::isDeleted() const
{
    return deleted;
}

size_t
Timer::getExpTime() const
{
    return expired_time;
}

bool
TimerCmp::operator()(const Timer *a, const Timer *b) const
{
    return a->getExpTime() > b->getExpTime();
}