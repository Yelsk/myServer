/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-21 19:01:56
 * @FilePath: /myWebServer-master/oldversion/0.3/Semaphore.cc
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