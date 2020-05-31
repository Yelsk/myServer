/*
 * @Author: GanShuang
 * @Date: 2020-05-25 17:39:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-25 19:09:42
 * @FilePath: /myWebServer-master/MutexLock.h
 */ 

#pragma once

#include <pthread.h>
#include <exception>
#include "NonCopyable.h"

class MutexLock : NonCopyable
{
public:
    MutexLock(){
        if(pthread_mutex_init(&m_mutex,  nullptr) != 0){
            throw std::exception();
        }
    };
    ~MutexLock(){
        pthread_mutex_lock(&m_mutex);
        pthread_mutex_destroy(&m_mutex);
    };
    pthread_mutex_t *get(){ return &m_mutex; };
    bool lock(){
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;

private:
    friend class Condition;
};

class MutexLockGuard : NonCopyable
{
public:
    explicit MutexLockGuard(MutexLock &mutex_) : m_mutex(mutex_) { m_mutex.lock(); }
    ~MutexLockGuard() { m_mutex.unlock(); }

private:
    MutexLock &m_mutex;
};