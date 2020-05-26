/*
 * @Author: GanShuang
 * @Date: 2020-05-25 21:12:44
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 12:20:15
 * @FilePath: /myWebServer-master/EventLoop.cc
 */ 

#pragma once

#include <assert.h>
#include "EventLoop.h"
#include "Log.h"

__thread EventLoop *t_loopInThisThread = 0;

int
createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        LOG_ERROR("failed in eventfd");
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : m_looping(false),
    m_poller(new Epoll()),
    m_wakeupFd(createEventfd()),
    m_quit(false),
    m_eventHandling(false),
    m_callingPendingFunc(false),
    m_wakeupChannel(new Channel(this, m_wakeupFd)),
    m_threadId(CurrentThread::tid())
{
    LOG_INFO("EventLoop created %p in thread %d", this, m_threadId);
    Log::get_instance()->flush();
    if(t_loopInThisThread)
    {
        LOG_ERROR("Another EventLoop %p exists in this thread %d", t_loopInThisThread, m_threadId);
    }
    else
    {
        t_loopInThisThread = this;
    }
    m_wakeupChannel->setEvents(EPOLLIN | EPOLLET);
    m_wakeupChannel->setReadHandler(bind(&EventLoop::handleRead, this));
    m_wakeupChannel->setConnHandler(bind(&EventLoop::handleConn, this));
    m_poller->epoll_add(m_wakeupChannel, 0);
}

EventLoop::~EventLoop()
{
    assert(!m_looping);
    close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}

void
EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(m_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8", n);
    }
}

void
EventLoop::loop()
{
    assert(!m_looping);
    assertInLoopThread();
    m_looping = true;
    m_quit = false;
    std::vector<Channel *> ret;
    while(!quit){
        ret.clear();
        ret = m_poller->poll();
        m_eventHandling = true;
        for(auto &it : ret) it->handleEvents();
        m_eventHandling = false;
        doPendingFuncs();
        m_poller->handleExpired();
    }
    LOG_INFO("EventLoop %p stop looping", this);
    Log::get_instance()->flush();
    m_looping = false;
}

void
EventLoop::doPendingFuncs()
{
    std::vector<Functor> funcs;
    m_callingPendingFunc = true;
    {
        MutexLockGuard lock(m_mutex);
        funcs.swap(m_pendingFuncs);
    }
    for(size_t i = 0; i < funcs.size(); i++) funcs[i]();
    m_callingPendingFunc = false;
}

void
EventLoop::quit()
{
    m_quit = true;
    if(!isInLoopThread()){
        wakeup();
    }
}

void
EventLoop::handleConn()
{
    m_poller->epoll_mod(m_wakeupChannel, 0);
}

void
EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
    }
    m_wakeupChannel->setEvents(EPOLLIN | EPOLLET);
}

void
EventLoop::runInLoop(Functor &&func)
{
    if(isInLoopThread()){
        func();
    }
    else
        queueInLoop(std::move(func));
}

void
EventLoop::queueInLoop(Functor &&func)
{
    {
        MutexLockGuard lock(m_mutex);
        m_pendingFuncs.emplace_back(std::move(func));
    }
    if(!isInLoopThread() || m_callingPendingFunc) wakeup();
}