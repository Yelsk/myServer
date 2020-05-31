/*
 * @Author: GanShuang
 * @Date: 2020-05-26 16:47:47
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 21:18:28
 * @FilePath: /myWebServer-master/EventLoopThread.h
 */ 

#pragma once


#include "Condition.h"
#include "MutexLock.h"
#include "Thread.h"
#include "NonCopyable.h"
#include "EventLoop.h"

class EventLoopThread : NonCopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc();
    Thread m_thread;
    EventLoop *m_loop;
    bool m_exiting;
    MutexLock m_mutex;
    Condition m_cond;
};
