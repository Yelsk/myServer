/*
 * @Author: GanShuang 
 * @Date: 2020-05-08 21:22:26 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 23:10:32
 */

#include "Epoll.h"
#include "HttpConnection.h"
#include "Timer.h"

const int MAXEPOLL = 1000;

Epoll::Epoll()
                :epfd(epoll_create(MAXEPOLL))
{
    std::cout << "epfd = " << epfd << std::endl;
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
Epoll::epoll_add(HttpConnection *conn)
{
    std::cout << "epoll_add" << std::endl;
    int fd = conn->getFd();
    std::cout << "add fd" << fd << std::endl;
    struct epoll_event event;
    event.data.ptr = (void *)conn;
    event.events = conn->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        return 0;
    }
    std::cout << "epoll_add success" << std::endl;
    return 1;
}

int
Epoll::epoll_mod(HttpConnection *conn)
{
    int fd = conn->getFd();
    struct epoll_event event;
    event.data.ptr = (void *)conn;
    event.events = conn->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("epoll_mod error");
        return 0;
    }
    return 1;
}

int
Epoll::epoll_del(HttpConnection *conn)
{
    int fd = conn->getFd();
    struct epoll_event event;
    event.data.ptr = (void *)conn;
    event.events = conn->getEvents();
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
        return 0;
    }
    return 1;
}

int
Epoll::my_epoll_wait(struct epoll_event *events, int max_events, int timeout)
{
    int ret_count = epoll_wait(epfd, events, max_events, timeout);
    if(ret_count < 0){
        perror("epoll wait error");
    }
    return ret_count;
}