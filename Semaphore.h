/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 17:50:21 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:00:18
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <exception>
#include <semaphore.h>
#include<pthread.h>
#include "HttpConnection.h"

class Semaphore
{
public:
    Semaphore(){
        if(sem_init(&sem_, 0, 0) != 0){
            throw std::exception();
        }
    };
    ~Semaphore(){
        sem_destroy(&sem_);
    };
    bool wait();
    bool post();

private:
    sem_t sem_;
};

#endif