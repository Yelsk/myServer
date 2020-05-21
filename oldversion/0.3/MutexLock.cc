/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-21 19:01:36
 * @FilePath: /myWebServer-master/oldversion/0.3/MutexLock.cc
 */ 

#include "MutexLock.h"

bool
MutexLock::lock(){
    return pthread_mutex_lock(&mutex_) == 0;
}

bool
MutexLock::unlock(){
    return pthread_mutex_unlock(&mutex_) == 0;
}