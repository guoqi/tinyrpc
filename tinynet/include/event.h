/**
 * File: event.h
 * Author: qiguo
 * Date: 2017.12.10 
 */
#ifndef __TINYNET_EVENT_H__
#define __TINYNET_EVENT_H__

#include <sys/epoll.h>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <queue>
#include <sig.h>
#include <assert.h>
#include "util.h"

namespace tinynet
{
    /*
     * Event type
     */
    /*
    enum class EventType {
        EV_SOCK        = 0,
        EV_TIMER       = 1,
        EV_SIGNAL      = 2
    };
     */

    class EventLoop;

    class Event;

    /*
     * Generic event callback function type.
     * @param   loop      ---    current event loop 
     *          ev        ---    current event
     *          T data    ---    user-side data
     * @return           ---    succ or fail (return errcode)
     */
    using EventCallback = std::function<void(EventLoop & loop, Event & ev)>;
    using TimerCallback = std::function<void(EventLoop & loop)>;

    /*
     * Empty callback functor
     */
    class EmptyCallback
    {
        public:
            void operator() (EventLoop & loop, Event & ev) { }
    };

    /*
     * A base event.
     * It is attached an action which acts when it is triggered.
     * Succ action will be called first and then fail action if the former fails.
     */
    class Event
    {
        public:
            explicit Event(int fd, bool readable = false, bool writeable = false)
                : m_fd(fd), m_readable(readable), m_writeable(writeable),
                  m_readcb(EmptyCallback()), m_writecb(EmptyCallback()) {}

            Event() : Event(-1) {}

            Event(const Event & ev) = default;

            virtual ~Event() = default;

            Event & onRead(const EventCallback & readcb) { m_readcb = readcb; m_readable = true; return *this; }
            Event & onWrite(const EventCallback & writecb) { m_writecb = writecb; m_writeable = true; return *this; }

            void onRead(EventLoop & loop) { return m_readcb(loop, *this); }
            void onWrite(EventLoop & loop) { return m_writecb(loop, *this); }

            inline void attach(int fd) { m_fd = fd; }
            inline int fd() const { return m_fd; }
            inline void readable(bool readable) { m_readable = readable; }
            inline void writeable(bool writeable) { m_writeable = writeable; }
            inline bool readable() const { return m_readable; }
            inline bool writeable() const { return m_writeable; }

        private:
            int                             m_fd;       // a socket file descriptor
            bool                            m_readable; // read able
            bool                            m_writeable; // write able
            EventCallback                   m_readcb;   // read callback
            EventCallback                   m_writecb;  // wiret callback
    };

    /*
     * Simple event loop.
     */
    class EventLoop : public util::noncopyable
    {
        public:
            explicit EventLoop(int maxnum);

            // alter or update(if event has already exists) an event
            int alter(const Event &ev);

            // remove an event
            int remove(int fd);

            int remove(const Event & ev);

            //  start event loop
            void start();

            // stop event loop
            void stop() { m_stop = true; }

            // judge if the loop is stopped
            bool isstop() { return m_stop; }

            // timer
            void runAfter(int64_t mils, const TimerCallback &cb, int64_t interval = 0)
            {
                m_timers.push(Timer(util::Time::nowMs() + mils, cb, interval));
            }

            void runAt(int64_t mils, const TimerCallback & cb, int64_t interval = 0)
            {
                m_timers.push(Timer(mils, cb, interval));
            }

        private:
            // loop only once
            void loopOnce();

            epoll_event convert(const Event & ev);

        private:
            class Timer
            {
            public:
                struct Comparison
                {
                    bool operator() (const Timer & lhs, const Timer & rhs) const
                    {
                        return lhs.time() < rhs.time();
                    }
                };

            public:
                Timer(int64_t tm, const TimerCallback & cb, int64_t inval): m_time(tm), m_interval(inval), m_action(cb) {}
                ~Timer() = default;
                inline int64_t time() const { return m_time; }
                inline int64_t interval() const { return m_interval; }
                inline const TimerCallback & action() const { return m_action; }

                void operator() (EventLoop & loop) const { m_action(loop); }

            private:
                int64_t         m_time;
                int64_t         m_interval;
                TimerCallback   m_action;
            };

        private:
            int                                             m_epfd;         // epoll fd
            bool                                            m_stop;         // event loop stop flag

            std::priority_queue<Timer,
                        std::vector<Timer>,
                        Timer::Comparison>                  m_timers;   // timer events
            std::vector<Timer>                              m_readyTimers;  // ready timer events
            std::map< int , Event >                         m_events;   // socket events

            int                                             m_maxnum;   // max number of events
    };
}

#endif