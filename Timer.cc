/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 14:52:52
 * @FilePath: /myWebServer-master/Timer.cc
 */ 

#include "Timer.h"
#include "HttpConnection.h"

using namespace std;

Timer::Timer(HttpConnection *conn_, int timeout)
    : m_deleted(false), 
    m_conn(conn_)
{
    //cout << "myTimer()" << endl;
    struct timeval now;
    gettimeofday(&now, nullptr);
    // 以毫秒计
    expired_time = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

Timer::~Timer()
{
    //cout << "~myTimer" << endl;
    if(m_conn)
    {
        //cout << "connection = " << conn << endl;
        delete m_conn;
        m_conn = nullptr;
    }
}

void
Timer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    expired_time = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

void
Timer::setDeleted()
{
    m_deleted = true;
}

bool
Timer::isValid()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    size_t tmp = ((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000);
    if(tmp < expired_time)
    {
        return true;
    }
    else
    {
        this->setDeleted();
        return false;
    }
}

void
Timer::clearConn()
{
    m_conn = nullptr;
    this->setDeleted();
}

bool
Timer::isDeleted() const
{
    return m_deleted;
}

size_t
Timer::getExpTime() const
{
    return expired_time;
}

bool
TimerCmp::operator()(const Timer *a, const Timer *b) const
{
    return a->getExpTime() > b->getExpTime();
}

TimerQueue::TimerQueue(){}

TimerQueue::~TimerQueue()
{
    Timer *timer;
    while(!m_queue.empty())
    {
        timer = m_queue.top();
        m_queue.pop();
        delete timer;
        timer = nullptr;
    }
}

void
TimerQueue::addTimer(Timer *timer)
{
    if(timer) m_queue.push(timer);
    else{
        perror("no timer");
    }
    return;
}

void
TimerQueue::addTimer(shared_ptr<HttpConnection> conn_, int timeout)
{
    Timer *timer = new Timer(conn_, timeout);
    m_queue.push(timer);
}

void
TimerQueue::handleExpired()
{
    while(!m_queue.empty())
    {
        Timer *timer = m_queue.top();
        if(timer->isDeleted()){
            m_queue.pop();
            delete timer;
        }
        else if(!timer->isValid()){
            m_queue.pop();
            delete timer;
        }
        else{
            break;
        }
    }
}