/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 19:14:22 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:04:50
 */

#ifndef CONDITION_H
#define CONDITION_H

#include <exception>
#include <pthread.h>
#include "HttpConnection.h"
#include "MutexLock.h"

class MutexLock;

class Condition
{
public:
    explicit Condition(MutexLock &mutex)
        :mutex_(mutex)
    {
        if(pthread_cond_init(&cond_, nullptr) != 0)
        {
            throw std::exception();
        }
    };
    ~Condition()
    {
        pthread_cond_destroy(&cond_);
    };
    bool wait();
    bool notify();
private:
    MutexLock mutex_;
    pthread_cond_t cond_;
};

#endif