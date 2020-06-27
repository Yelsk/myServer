/*
 * @Author: GanShuang
 * @Date: 2020-05-25 19:38:20
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 19:36:09
 * @FilePath: /myWebServer-master/CountDownLatch.h
 */ 

#pragma once

#include "MutexLock.h"
#include "Condition.h"
#include "NonCopyable.h"

/**
 * @description: CountDownLatch的主要作用是确保Thread中
 * 传进去的func真的启动了以后外层的start才返回
 */
class CountDownLatch : NonCopyable
{
public:
    explicit CountDownLatch(int count)
                                                        : m_mutex(),
                                                        m_cond(m_mutex),
                                                        m_count(count) {}
    ~CountDownLatch() {}
    void wait() {
        MutexLockGuard lock(m_mutex);
        while(m_count > 0) m_cond.wait();
    }
    void countDown() {
        MutexLockGuard lock(m_mutex);
        m_count--;
        if(m_count == 0) m_cond.notifyAll();
    }

private:
    mutable MutexLock m_mutex;
    Condition m_cond;
    int m_count;
};