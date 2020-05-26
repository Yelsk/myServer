/*
 * @Author: GanShuang
 * @Date: 2020-05-25 22:29:25
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 09:53:28
 * @FilePath: /myWebServer-master/Channel.cc
 */ 

#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop_)
    : m_loop(loop_),
    m_events(0),
    m_fd(0)
{
}

Channel::Channel(EventLoop *loop_, int fd_)
    : m_loop(loop_),
    m_events(0),
    m_fd(fd_)
{
}

Channel::~Channel()
{
}

void
Channel::handleRead()
{
    if(m_readHandler)
    {
        m_readHandler();
    }
}

void
Channel::handleWrite()
{
    if(m_writeHandler)
    {
        m_writeHandler();
    }
}

void
Channel::handleConn()
{
    if(handleConn)
    {
        m_connHandler();
    }
}