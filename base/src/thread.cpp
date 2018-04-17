//
// Created by qiguo on 3/17/18.
//

#include "thread.h"

using namespace util;
using namespace std;

namespace tinyrpc
{

    std::shared_ptr<Thread> Thread::create(ThreadFunc func)
    {
        auto thread = make_shared<Thread>(func);
        thread->run();
        return thread;
    }

    Thread::Thread(ThreadFunc func)
        : m_func(std::move(func)), m_stop(false)
    {
        int ret = pthread_create(&m_thread, nullptr, threadFunc, this);
        fatalif(ret != 0);
    }

    Thread::~Thread()
    {
        stop();
        join(); // wait for thread closing
    }

    void Thread::run()
    {
        m_cond.signal();
    }

    void Thread::join()
    {
        pthread_join(m_thread, nullptr);
    }

    void* Thread::threadFunc(void * param)
    {
        auto self = (Thread *)param;
        while (! self->m_stop)
        {
            (self->m_func)();

            self->m_cond.wait();
        }
    }


    ThreadCond::ThreadCond()
        : m_signal(false)
    {
        pthread_mutex_init(&m_mtx, nullptr);
        pthread_cond_init(&m_cond, nullptr);
    }

    ThreadCond::~ThreadCond()
    {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mtx);
    }

    void ThreadCond::wait() noexcept
    {
        pthread_mutex_lock(&m_mtx);
        while (! m_signal) {
            pthread_cond_wait(&m_cond, &m_mtx);
        }
        m_signal = false;
        pthread_mutex_unlock(&m_mtx);
    }

    void ThreadCond::signal() noexcept
    {
        pthread_mutex_lock(&m_mtx);
        m_signal = true;
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mtx);
    }


    ThreadMutex::ThreadMutex()
    {
        pthread_mutex_init(&m_mtx, nullptr);
        pthread_mutex_lock(&m_mtx);
    }

    ThreadMutex::~ThreadMutex()
    {
        pthread_mutex_unlock(&m_mtx);
        pthread_mutex_destroy(&m_mtx);
    }
}
