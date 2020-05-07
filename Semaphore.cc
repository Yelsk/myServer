/*
 * @Author: GanShuang 
 * @Date: 2020-05-05 17:54:41 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-05 20:00:26
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