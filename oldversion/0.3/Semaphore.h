/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-22 09:50:24
 * @FilePath: /myWebServer-master/oldversion/0.3/Semaphore.h
 */ 

#pragma once

#include <exception>
#include <semaphore.h>

class Semaphore
{
public:
    Semaphore(){
        if(sem_init(&sem_, 0, 0) != 0){
            throw std::exception();
        }
    };
    Semaphore(int num){
        if(sem_init(&sem_, 0, num) != 0){
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