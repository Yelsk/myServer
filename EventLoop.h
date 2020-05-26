/*
 * @Author: GanShuang
 * @Date: 2020-05-25 11:32:50
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 17:20:54
 * @FilePath: /myWebServer-master/EventLoop.h
 */ 

#pragma once

#include <vector>
#include <memory>
#include <sys/eventfd.h>
#include <assert.h>
#include "Channel.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "Epoll.h"
#include "Util.h"

class EventLoop : NonCopyable
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void runInLoop(Functor &&func);
    void queueInLoop(Functor &&func);
    void shutdown(Channel *channel) { shutDownWR(channel->getFd()); }
    void assertInLoopThread() { assert(isInLoopThread()); }
    bool isInLoopThread() { return m_threadId == CurrentThread::tid(); }

private:
    bool m_looping;
    bool m_quit;
    bool m_eventHandling;
    bool m_callingPendingFunc;
    const pid_t m_threadId;
    Epoll *m_poller;
    Channel *m_wakeupChannel;
    int m_wakeupFd;
    mutable MutexLock m_mutex;
    std::vector<Functor> m_pendingFuncs;
    void wakeup();
    void handleRead();
    void doPendingFuncs();
    void handleConn();
};