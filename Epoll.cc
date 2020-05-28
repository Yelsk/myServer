/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-28 15:32:07
 * @FilePath: /myWebServer-master/Epoll.cc
 */ 

#include "Epoll.h"
#include "HttpConnection.h"
#include "Channel.h"

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll()
    : epfd(epoll_create1(EPOLL_CLOEXEC)),
    m_events(EVENTSNUM),
    m_channels(MAXFDS, nullptr)
{
    assert(epfd > 0);
}

Epoll::~Epoll()
{
    close(epfd);
}

int
Epoll::epoll_add(int fd, int events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        return 0;
    }
    return 1;
}

int
Epoll::epoll_add(Channel *request, int timeout)
{
    int fd = request->getFd();
    if(timeout > 0){
        add_timer(request, timeout);
        m_conns[fd] = request->getConn();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    m_channels[fd] = request;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        delete m_channels[fd];
        return 0;
    }
    return 1;
}

int
Epoll::epoll_mod(Channel *request, int timeout)
{
    if(timeout > 0) add_timer(request, timeout);
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("epoll_mod error");
        delete m_channels[fd];
        return 0;
    }
    return 1;
}

int
Epoll::epoll_del(Channel *request)
{
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
        return 0;
    }
    delete m_channels[fd];
    return 1;
}

std::vector<Channel *>
Epoll::poll()
{
    int ret_count = epoll_wait(epfd, &*m_events.begin(), m_events.size(), EPOLLWAIT_TIME);
    if(ret_count < 0){
        perror("epoll wait error");
    }
    std::vector<Channel *> req_data = getEventsRequest(ret_count);
    return req_data;
}

std::vector<Channel *>
Epoll::getEventsRequest(int events_count)
{
    std::vector<Channel *> req_data;
    for(int i = 0; i < events_count; i++)
    {
        int fd = m_events[i].data.fd;
        Channel *cur_req = m_channels[fd];
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
        if(req_data.size() > 0) return req_data;
    }
}

void
Epoll::add_timer(Channel *req_data, int timeout)
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