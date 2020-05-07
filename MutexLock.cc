/*
 * @Author: GanShuang
 * @Date: 2020-05-05 17:20:24 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:00:49
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