/*
 * @Author: GanShuang
 * @Date: 2020-05-25 11:32:50
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-03 11:00:17
 * @FilePath: /myWebServer-master/muduo_style/0.1/EventLoop.h
 */ 

#pragma once

#include <vector>
#include <memory>
#include <sys/eventfd.h>
#include <assert.h>
#include <functional>
#include "Channel.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "Epoll.h"
#include "Util.h"

using std::shared_ptr;
using std::vector;

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
    void shutdown(shared_ptr<Channel> channel) { shutdownwr(channel->getFd()); }
    void assertInLoopThread() { assert(isInLoopThread()); }
    bool isInLoopThread() { return m_threadId == CurrentThread::tid(); }
    void removeFromPoller(shared_ptr<Channel> channel) {
        m_poller->epoll_del(channel);
    }
    void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {
        m_poller->epoll_mod(channel, timeout);
    }
    void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {
        m_poller->epoll_add(channel, timeout);
    }
private:
    bool m_looping;
    bool m_quit;
    bool m_eventHandling;
    bool m_callingPendingFunc;
    const pid_t m_threadId;
    //声明顺序m_wakeupFd > m_wakeupChannel
    //否则m_wakeupChannel(new Channel(this, m_wakeupFd))会先于m_wakeupFd初始化
    int m_wakeupFd;
    Epoll *m_poller;
    shared_ptr<Channel> m_wakeupChannel;
    mutable MutexLock m_mutex;
    vector<Functor> m_pendingFuncs;
    void wakeup();
    void handleRead();
    void doPendingFuncs();
    void handleConn();
};