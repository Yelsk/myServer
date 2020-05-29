/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-29 21:28:45
 * @FilePath: /myWebServer-master/Timer.cc
 */ 

#include "Timer.h"
#include "HttpConnection.h"

using namespace std;

Timer::Timer(shared_ptr<HttpConnection> conn_, int timeout)
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
    if(m_conn) m_conn->HandleClose();
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
    m_conn.reset();
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
TimerCmp::operator()(const shared_ptr<Timer> a, const shared_ptr<Timer> b) const
{
    return a->getExpTime() > b->getExpTime();
}

TimerQueue::TimerQueue(){}

TimerQueue::~TimerQueue()
{
    shared_ptr<Timer> timer;
    while(!m_queue.empty())
    {
        timer = m_queue.top();
        m_queue.pop();
    }
}

void
TimerQueue::addTimer(shared_ptr<HttpConnection> conn_, int timeout)
{
    shared_ptr<Timer> timer(new Timer(conn_, timeout));
    m_queue.push(timer);
    conn_->linkTimer(timer);
}

void
TimerQueue::handleExpired()
{
    while(!m_queue.empty())
    {
        shared_ptr<Timer> timer = m_queue.top();
        if(timer->isDeleted()){
            m_queue.pop();
        }
        else if(!timer->isValid()){
            m_queue.pop();
        }
        else{
            break;
        }
    }
}