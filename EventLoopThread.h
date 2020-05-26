/*
 * @Author: GanShuang
 * @Date: 2020-05-26 16:47:47
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 17:38:26
 * @FilePath: /myWebServer-master/EventLoopThread.h
 */ 

#pragma once

#include "EventLoop.h"
#include "Condition.h"
#include "MutexLock.h"
#include "Thread.h"
#include "NonCopyable.h"

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
