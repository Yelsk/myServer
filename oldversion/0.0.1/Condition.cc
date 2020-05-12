/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 19:14:18 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:05:52
 */

#include "Condition.h"

bool
Condition::wait(){
    int ret;
    mutex_.lock();
    ret = pthread_cond_wait(&cond_, mutex_.GetPthreadMutex());
    mutex_.unlock();
    return ret == 0;
}

bool
Condition::notify(){
    return pthread_cond_signal(&cond_) == 0;
}