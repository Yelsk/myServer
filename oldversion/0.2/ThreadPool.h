/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 20:07:25 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-07 17:23:51
 */

#pragma once

#include <iostream>
#include <exception>
#include<list>
#include<cstdio>
#include "MutexLock.h"
#include "Semaphore.h"

using namespace std;

template<typename T>
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();
    bool addjob(T* request);
private:
    void run();
    static void *pthread_func(void *arg);
    int thread_num;
    int jobthread_num;
    pthread_t *pthread_pool;
    std::list<T *> workqueue_   ;
    MutexLock mutex_;
    Semaphore sem_;
    bool quit_;
};
 
template <typename T>
ThreadPool<T>::ThreadPool(){
    thread_num = 4;
    jobthread_num = 10000;
    quit_ = false;
    pthread_pool = new pthread_t[thread_num];
    if(!pthread_pool){
        throw exception();
    }
    for(int i = 0; i < thread_num; i++){
        if(pthread_create(pthread_pool+i, nullptr, pthread_func, this) != 0){
            delete [] pthread_pool;
            throw exception();
        }
        if(pthread_detach(pthread_pool[i])){
            delete [] pthread_pool;
            throw exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool(){
    delete [] pthread_pool;
    quit_ = true;
}

template <typename T>
bool ThreadPool<T>::addjob(T* request){
    mutex_.lock();
    if(workqueue_.size() > jobthread_num){
        mutex_.unlock();
        return false;
    }
    workqueue_.push_back(request);
    mutex_.unlock();
    sem_.post();
    return true;
}

template <typename T>
void *ThreadPool<T>::pthread_func(void *arg){
    ThreadPool *pool = (ThreadPool *) arg;
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run(){
    while(!quit_){
        sem_.wait();
        mutex_.lock();
        if(workqueue_.empty()){
            mutex_.unlock();
            continue;
        }
        T *request = workqueue_.front();
        workqueue_.pop_front();
        mutex_.unlock();
        if(!request) continue;
        request->doit();
    }
}