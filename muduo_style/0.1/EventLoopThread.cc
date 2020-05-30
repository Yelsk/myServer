/*
 * @Author: GanShuang
 * @Date: 2020-05-26 16:47:58
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 21:59:44
 * @FilePath: /myWebServer-master/EventLoopThread.cc
 */ 

#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
    : m_loop(nullptr),
    m_exiting(false),
    m_thread(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
    m_mutex(),
    m_cond(m_mutex)
{
}

EventLoopThread::~EventLoopThread()
{
    m_exiting = true;
    if(m_loop)
    {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop *
EventLoopThread::startLoop()
{
    assert(!m_thread.started());
    m_thread.start();
    {
        MutexLockGuard lock(m_mutex);
        while (!m_loop) m_cond.wait();
    }
    return m_loop;
}

void
EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(m_mutex);
        m_loop = &loop;
        m_cond.notify();
    }
    loop.loop();
    m_loop = nullptr;
}