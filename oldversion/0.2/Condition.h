/*
 * @Author: GanShuang 
 * @Date: 2020-05-16 20:01:50 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-16 20:07:30
 */

#pragma once

#include <exception>
#include <pthread.h>

class Condition
{
public:
    Condition();
    ~Condition();
    bool wait(pthread_mutex_t *m_mutex);
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t);
    bool signal();
    bool broadcast();
private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};