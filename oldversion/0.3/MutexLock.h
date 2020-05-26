/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-25 17:30:15
 * @FilePath: /myWebServer-master/oldversion/0.3/MutexLock.h
 */ 

#pragma once

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
    pthread_mutex_t *get(){ return &mutex_; };
    bool lock(){
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mutex_) == 0;
    }
private:
    pthread_mutex_t mutex_;
};