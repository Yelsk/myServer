/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 20:07:09 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 20:38:27
 */

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include "HttpConnection.h"

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
private:
    bool operator()(const Timer *a, const Timer *b) const;
public:
    TimerCmp();
    ~TimerCmp();
};

#endif