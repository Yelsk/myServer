/*
 * @Author: GanShuang
 * @Date: 2020-05-26 17:42:02
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 21:59:36
 * @FilePath: /myWebServer-master/EventLoopThreadPool.cc
 */ 

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : m_loop(baseLoop),
    m_started(false),
    m_numThreads(numThreads),
    m_next(0)
{
    if(m_numThreads <= 0)
    {
        LOG_ERROR("ThreadsNum <= 0");
        abort();
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    for(int i = 0; i < m_numThreads; i++)
    {
        EventLoopThread *thread = m_threads[i];
        delete thread;
        thread = nullptr;
    }
}

void
EventLoopThreadPool::start()
{
    m_loop->assertInLoopThread();
    m_started = true;
    for(int i = 0; i < m_numThreads; i++)
    {
        EventLoopThread *thread = new EventLoopThread();
        m_threads.push_back(thread);
        m_loops.push_back(thread->startLoop());
    }
}

EventLoop *
EventLoopThreadPool::getNextLoop()
{
    m_loop->assertInLoopThread();
    assert(m_started);
    EventLoop *loop = m_loop;
    if(!m_loops.empty())
    {
        loop = m_loops[m_next];
        m_next = (m_next + 1) % m_numThreads;
    }
    return loop;
}