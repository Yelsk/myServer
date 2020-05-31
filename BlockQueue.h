/*
 * @Author: your name
 * @Date: 2020-05-16 19:54:29
 * @LastEditTime: 2020-05-27 19:09:19
 * @LastEditors: GanShuang
 * @Description: In User Settings Edit
 * @FilePath: /myWebServer-master/BlockQueue.h
 */ 


#pragma once

#include <iostream>
#include <queue>
#include <string>
#include <stdlib.h>
#include <sys/time.h>
#include "MutexLock.h"
#include "Condition.h"

using namespace std;

template<class T>
class BlockQueue
{
public:
    BlockQueue(int max_size = 1000)
        :m_cond(m_mutex)
    {
        if(max_size <= 0)
        {
            exit(-1);
        }
        m_max_size = max_size;
        m_size = 0;
    }
    ~BlockQueue() {}

    void clear()
    {
        m_mutex.lock();
        m_size = 0;
        m_queue.clear();
        m_mutex.unlock();
        return;
    }

    bool full()
    {
        m_mutex.lock();
        if(m_size >= m_max_size)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool empty()
    {
        m_mutex.lock();
        if(m_size == 0)
        {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }

    bool front(T &value)
    {
        m_mutex.lock();
        if(m_size == 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_queue.front();
        m_mutex.unlock();
        return true;
    }

    bool back(T &value)
    {
        m_mutex.lock();
        if(m_size == 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_queue.back();
        m_mutex.unlock();
        return true;
    }

    int size() 
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_size;

        m_mutex.unlock();
        return tmp;
    }

    int max_size()
    {
        int tmp = 0;

        m_mutex.lock();
        tmp = m_max_size;

        m_mutex.unlock();
        return tmp;
    }

    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列,相当于生产者生产了一个元素
    //若当前没有线程等待条件变量,则唤醒无意义
    bool push(const T &item)
    {

        m_mutex.lock();
        if (m_size >= m_max_size)
        {

            m_cond.notifyAll();
            m_mutex.unlock();
            return false;
        }

        m_queue.push_back(item);

        m_size++;

        m_cond.notifyAll();
        m_mutex.unlock();
        return true;
    }

    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T &item)
    {

        m_mutex.lock();
        while (m_size <= 0)
        {
            
            if (!m_cond.wait())
            {
                m_mutex.unlock();
                return false;
            }
        }
        item = m_queue.front();
        m_queue.pop_front();
        m_size--;
        m_mutex.unlock();
        return true;
    }

private:
    MutexLock m_mutex;
    Condition m_cond;
    deque<T> m_queue;
    int m_size;
    int m_max_size;
};