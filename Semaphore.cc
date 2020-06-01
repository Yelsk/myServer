/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-01 22:28:27
 * @FilePath: /myWebServer-master/Semaphore.cc
 */ 

#include "Semaphore.h"

bool
Semaphore::wait(){
    return sem_wait(&sem_) == 0;
}

bool
Semaphore::post(){
    return sem_post(&sem_) == 0;
}