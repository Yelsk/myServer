/*
 * @Author: GanShuang
 * @Date: 2020-05-26 17:41:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-29 22:36:22
 * @FilePath: /myWebServer-master/EventLoopThreadPool.h
 */ 

#pragma once

#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "Log.h"
#include "NonCopyable.h"

class EventLoopThreadPool : NonCopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
    ~EventLoopThreadPool();
    void start();
    EventLoop *getNextLoop();

private:
    EventLoop *m_loop;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::shared_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop *> m_loops;
};