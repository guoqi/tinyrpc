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

    Thread::~Thread()
    {
        pthread_cancel(m_thread);   // cancel thread.
    }

    void Thread::run()
    {
        int ret = pthread_create(&m_thread, nullptr, threadFunc, this);
        fatalif(ret != 0);
    }

    void Thread::join()
    {
        pthread_join(m_thread, nullptr);
    }

    void* Thread::threadFunc(void * param)
    {
        auto self = (Thread *)param;
        (self->m_func)();
    }

}
