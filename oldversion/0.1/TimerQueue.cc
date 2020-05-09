/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 21:12:31 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 21:19:39
 */

#include "TimerQueue.h"

TimerQueue::TimerQueue(){}

TimerQueue::~TimerQueue()
{
    Timer *timer;
    while(!timerQueue.empty())
    {
        timer = timerQueue.top();
        timerQueue.pop();
        delete timer;
        timer = nullptr;
    }
}

void
TimerQueue::addTimer(Timer *timer)
{
    timerQueue.push(timer);
}

void
TimerQueue::addTimer(HttpConnection *_conn, int timeout)
{
    Timer *timer = new Timer(_conn, timeout);
    timerQueue.push(timer);
}