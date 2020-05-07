/*
 * @Author: GanShuang
 * @Date: 2020-05-05 17:20:24 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:00:49
 */

#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include <exception>
#include <pthread.h>

class MutexLock
{
public:
    MutexLock(){
        if(pthread_mutex_init(&mutex_, nullptr) != 0){
            throw std::exception();
        }
    };
    ~MutexLock(){
        pthread_mutex_destroy(&mutex_);
    };
    pthread_mutex_t *GetPthreadMutex(){ return &mutex_; };
    bool lock();
    bool unlock();
private:
    pthread_mutex_t mutex_;
};

#endif