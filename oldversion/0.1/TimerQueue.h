/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 21:06:06 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 21:20:43
 */

#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <queue>
#include "Timer.h"
#include "HttpConnection.h"

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

#endif