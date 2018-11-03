//
// Created by qiguo on 4/17/18.
//

#ifndef __TINYRPC_TS_QUEUE_H__
#define __TINYRPC_TS_QUEUE_H__

#include <stdexcept>
#include <cstdio>
#include <atomic>
#include <list>
#include "thread.h"

/*
 * Thread-safe unlimited size queue without lock
 * Support push, front, pop, size, empty operation
 * Implement by single linked list
 */

namespace tinyrpc
{
    template<typename T>
    class TSQueue
    {

        struct QNode
        {
            std::atomic<QNode *> next;
            T                    data;
        };

    public:
        TSQueue() : m_phead(nullptr), m_ptail(nullptr)
        {
            m_dummy = new QNode;
            m_dummy->next = nullptr;

            m_phead = m_dummy;
            m_ptail.store(m_dummy);
        }

        ~TSQueue()
        {
            QNode * p = m_phead->next.load();
            while (p != nullptr)
            {
                QNode * tmp = p;
                p = p->next;
                delete tmp;
            }

            delete m_dummy;
        }

        void push(const T & data)
        {
            auto p = new QNode;
            p->data = data;
            p->next.store(nullptr);

            QNode * orig_tail = nullptr;

            do
            {
                orig_tail = m_ptail.load(std::memory_order_acquire);
                // printf("[%ld]tail->next=%p, p=%p, tail=%p\n", pthread_self(), orig_tail->next.load(std::memory_order_relaxed), p, orig_tail);
            } while (! m_ptail.compare_exchange_strong(orig_tail, p, std::memory_order_release, std::memory_order_acquire));

            orig_tail->next.store(p, std::memory_order_release);
        }

        void pop()
        {
            QNode * p = nullptr;
            QNode * next = nullptr;

            do
            {
                next = m_phead->next.load(std::memory_order_acquire);
                if (next == nullptr)
                {
                    break;
                }
                p = next->next.load(std::memory_order_relaxed);
                if (p == nullptr)
                {
                    m_ptail.store(m_dummy, std::memory_order_release);
                }
            } while (! m_phead->next.compare_exchange_strong(next, p, std::memory_order_release, std::memory_order_acquire));

            delete next;   // in c++, delete a nullptr is allowed, so there is no need to check whether p is null or not.
        }

        T & front()
        {
            if (empty())
            {
                throw std::out_of_range("queue is empty!");
            }

            return m_phead->next.load(std::memory_order_acquire)->data;
        }

        const T & front() const
        {
            return front();
        }

        bool empty()
        {
            return m_phead->next.load(std::memory_order_acquire) == nullptr;
        }

    private:
        QNode           *     m_phead;   // always points to the next of dummy node (never changed)
        std::atomic<QNode *>  m_ptail;   // always points to the next of the tail node (whose dereference is always nullptr)
        QNode           *     m_dummy;   // dummy node, for simplify push and pop implementation
    };


    // thread-safe queue with lock and implementated with std::queue
    template<typename T>
    class TSLockQueue
    {
    public:
        TSLockQueue() = default;
        ~TSLockQueue() = default;

        bool empty()
        {
            ThreadGuard guard(m_mutex);
            return m_queue.empty();
        }

        typename std::list<T>::size_type size()
        {
            ThreadGuard guard(m_mutex);
            return m_queue.size();
        }

        typename std::list<T>::reference front()
        {
            ThreadGuard guard(m_mutex);
            return m_queue.front();
        }

        void push_back(const typename std::list<T>::value_type & val)
        {
            ThreadGuard guard(m_mutex);
            m_queue.push_back(val);
        }

        void pop_front()
        {
            ThreadGuard guard(m_mutex);
            if (m_queue.empty())
                return;
            m_queue.pop_front();
        }

        void remove(const typename std::list<T>::value_type & val)
        {
            ThreadGuard guard(m_mutex);
            m_queue.remove(val);
        }

    private:
        std::list<T>   m_queue;
        ThreadMutex    m_mutex;
    };
}

#endif //TINYRPC_TS_QUEUE_H
