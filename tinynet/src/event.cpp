/*
 * File: event.cpp
 * Author: qiguo
 * Date: 2017.12.10
 */

#include <set>
#include <sys/socket.h>
#include <algorithm>
#include "event.h"
#include "logger.h"

using namespace std;

namespace tinynet
{

EventLoop::EventLoop(int maxnum)
    : m_maxnum(maxnum), m_stop(false)
{
    m_epfd = epoll_create(maxnum);
    fatalif(m_epfd == -1);
}

// alter or update event
int EventLoop::alter(const Event &ev)
{
    epoll_event epoll_ev = convert(ev);

    if (m_events.find(ev.fd()) == m_events.end())
    {
        int r = epoll_ctl(m_epfd, EPOLL_CTL_ADD, ev.fd(), &epoll_ev);
        returnif(r, -1);
    }
    else
    {
        int r = epoll_ctl(m_epfd, EPOLL_CTL_MOD, ev.fd(), &epoll_ev);
        returnif(r, -1);
    }

    m_events[ev.fd()] = ev;

    return 0;
}

int EventLoop::remove(int fd)
{
    int r = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
    returnif(r, -1);

    m_events.erase(fd);

    return 0;
}

int EventLoop::remove(const Event & ev)
{
    remove(ev.fd());
}

epoll_event EventLoop::convert(const Event &ev)
{
    epoll_event epoll_ev;
    memset(&epoll_ev, 0, sizeof(epoll_ev));
    epoll_ev.events = EPOLLET;
    epoll_ev.data.fd = ev.fd();

    if (ev.readable())
    {
        epoll_ev.events |= EPOLLIN;
    }

    if (ev.writeable())
    {
        epoll_ev.events |= EPOLLOUT;
    }

    return epoll_ev;
}

void EventLoop::start()
{
    while (! isstop())
    {
        loopOnce();
    }
}

void EventLoop::loopOnce()
{
    epoll_event * events = new epoll_event[m_maxnum];

    int64_t timeout = 1000;
    int64_t now = util::Time::nowMs();

    if (! m_timers.empty())
    {
        for (timeout = m_timers.top().time() - now;
             ! m_timers.empty() && timeout <= 0;
             timeout = m_timers.top().time() - now)
        {
            m_readyTimers.push_back(m_timers.top());
            m_timers.pop();
        }
    }

    if (timeout > 1000) // ticks
    {
        timeout = 1000;
    }

    int n = epoll_wait(m_epfd, events, m_maxnum, timeout);
    fatalif(n == -1);

    for (int i=0; i<n; ++i)
    {
        epoll_event & ev = events[i];
        Event & event = m_events.at(ev.data.fd);

        if (event.writeable() && ev.events & EPOLLOUT) // socket is writeable
        {
            event.onWrite(*this);
        }

        if (event.readable() && ev.events & EPOLLIN) // socket is readable
        {
            event.onRead(*this);
        }
    }

    // timeout event
    if (0 == n)
    {
        // check if there are more overtime events
        while (! m_timers.empty() && m_timers.top().time() - now == timeout)
        {
            m_readyTimers.push_back(m_timers.top());
            m_timers.pop();
        }
    }

    // handle timer event
    std::for_each(m_readyTimers.begin(), m_readyTimers.end(),
                  [this](const Timer & tm){
                      if (tm.interval() != 0) {
                          runAfter(tm.interval(), tm.action(), tm.interval());
                      }
                      tm(*this);
                  }
    );
    m_readyTimers.clear();

    delete [] events;
}


}