/*
 * @Author: GanShuang
 * @Date: 2020-05-25 19:01:49
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-25 19:37:30
 * @FilePath: /myWebServer-master/Condition.h
 */ 

#pragma once

#include <pthread.h>
#include <exception>
#include "time.h"
#include "errno.h"
#include "MutexLock.h"
#include "NonCopyable.h"

class Condition : NonCopyable
{
public:
    explicit Condition(MutexLock &mutex_) : m_mutex(mutex_){
        if(pthread_cond_init(&m_cond, nullptr) != 0){
            throw std::exception();
        }
    }
    ~Condition() { pthread_cond_destroy(&m_cond); }
    bool wait() {
        return pthread_cond_wait(&m_cond,  m_mutex.get()) == 0;
    }
    bool notify() {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool notifyAll() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
    bool waitForSeconds(int seconds){
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.get(), &abstime);
    }

private:
    MutexLock &m_mutex;
    pthread_cond_t m_cond;
};