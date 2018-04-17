//
// Created by qiguo on 3/17/18.
//

#ifndef __TINYRPC_THREAD_H__
#define __TINYRPC_THREAD_H__

#include "logger.h"
#include "util.h"
#include <pthread.h>
#include <functional>

namespace tinyrpc
{

    using ThreadFunc = std::function<void(void)>;

    class ThreadCond
    {
    public:
        ThreadCond();
        ~ThreadCond();

        // wait until cond returns true
        void wait() noexcept;
        void signal() noexcept;

    private:
        pthread_cond_t      m_cond;
        pthread_mutex_t     m_mtx;
        bool                m_signal;
    };


    class ThreadMutex
    {
    public:
        ThreadMutex();
        ~ThreadMutex();

    private:
        pthread_mutex_t     m_mtx;
    };


    class Thread : util::noncopyable
    {
    public:
        static std::shared_ptr<Thread> create(ThreadFunc func);

        explicit Thread(ThreadFunc func);
        Thread() : Thread([](){}) {}
        ~Thread();  // exit thread

        void attach(ThreadFunc func) { m_func = std::move(func); }
        void run();
        void join();

        // stop and wakeup
        void stop() { m_stop = true; m_cond.signal(); }

    private:
        static void * threadFunc(void *);

    private:
        ThreadFunc          m_func;
        pthread_t           m_thread;
        bool                m_stop;
        ThreadCond          m_cond;
    };
}

#endif //TINYRPC_THREAD_H
