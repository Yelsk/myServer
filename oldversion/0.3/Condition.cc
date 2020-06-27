/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-21 19:00:04
 * @FilePath: /myWebServer-master/oldversion/0.3/Condition.cc
 */ 

#include "Condition.h"

Condition::Condition()
{
    if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
}

Condition::~Condition()
{
    pthread_cond_destroy(&m_cond);
}

bool 
Condition::wait(pthread_mutex_t *m_mutex)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&m_cond, m_mutex);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool 
Condition::timewait(pthread_mutex_t *m_mutex, struct timespec t)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool 
Condition::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}

bool 
Condition::broadcast()
{
    return pthread_cond_broadcast(&m_cond) == 0;
}