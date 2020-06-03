/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-03 10:58:45
 * @FilePath: /myWebServer-master/muduo_style/0.1/Epoll.cc
 */ 

#include <string>
#include "Epoll.h"
#include "HttpConnection.h"
#include "Channel.h"

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll()
    : epfd(epoll_create1(EPOLL_CLOEXEC)),
    m_events(EVENTSNUM)
{
    assert(epfd > 0);
}

Epoll::~Epoll()
{
    close(epfd);
}

int
Epoll::epoll_add(shared_ptr<Channel> request, int timeout)
{
    int fd = request->getFd();
    if(timeout > 0){
        add_timer(request, timeout);
        m_conns[fd] = request->getConn();
    }
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = fd;
    event.events = request->getEvents();
    m_channels[fd] = request;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        m_channels[fd].reset();
        return 0;
    }
    return 1;
}

int
Epoll::epoll_mod(shared_ptr<Channel> request, int timeout)
{
    if(timeout > 0) add_timer(request, timeout);
    int fd = request->getFd();
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = fd;
    event.events = request->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("epoll_mod error");
        m_channels[fd].reset();
        return 0;
    }
    return 1;
}

int
Epoll::epoll_del(shared_ptr<Channel> request)
{
    int fd = request->getFd();
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = fd;
    event.events = request->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
        return 0;
    }
    m_channels[fd].reset();
    m_conns[fd].reset();
    return 1;
}

vector<shared_ptr<Channel>>
Epoll::poll()
{
    int ret_count = epoll_wait(epfd, &*m_events.begin(), m_events.size(), EPOLLWAIT_TIME);
    if(ret_count < 0){
        perror("epoll wait error");
    }
    vector<shared_ptr<Channel>> req_data = getEventsRequest(ret_count);
    return req_data;
}

vector<shared_ptr<Channel>>
Epoll::getEventsRequest(int events_count)
{
    vector<shared_ptr<Channel>> req_data;
    for(int i = 0; i < events_count; i++)
    {
        int fd = m_events[i].data.fd;
        shared_ptr<Channel> cur_req = m_channels[fd];
        if(cur_req)
        {
            cur_req->setEvents(m_events[i].events);
            req_data.push_back(cur_req);
        }
        else
        {
            LOG_INFO("req is not valid");
            Log::get_instance()->flush();
        }
    }
    return req_data;
}

void
Epoll::add_timer(shared_ptr<Channel> req_data, int timeout)
{
    shared_ptr<HttpConnection> conn = req_data->getConn();
    if(conn)
    {
        timerQueue.addTimer(conn, timeout);
    }
    else
    {
        LOG_INFO("timer add failed!");
        Log::get_instance()->flush();
    }
    return;
}