//
// Created by qiguo on 3/17/18.
//

#ifndef __TINYRPC_THREAD_H__
#define __TINYRPC_THREAD_H__

#include "util.h"
#include <pthread.h>
#include <functional>

namespace tinyrpc
{

    using ThreadFunc = std::function<void(void)>;

    class Thread : util::noncopyable
    {
    public:
        static std::shared_ptr<Thread> create(ThreadFunc func);

        explicit Thread(ThreadFunc func) : m_func(std::move(func)) {}
        Thread() = default;
        ~Thread();  // exit thread

        void attach(ThreadFunc func) { m_func = std::move(func); }
        void run();
        void join();

    private:
        static void * threadFunc(void *);

    private:
        ThreadFunc          m_func;
        pthread_t           m_thread;
    };

}

#endif //TINYRPC_THREAD_H
