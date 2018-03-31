//
// Created by qiguo on 2/22/18.
//

#ifndef __TINYRPC_POOL_H__
#define __TINYRPC_POOL_H__

#include <cstdint>
#include <list>
#include <set>

namespace tinyrpc
{
    /**
     * Base common pool for any presist instance.
     * The instance must have copy constructor and move constructor and copyable
     */
    template<typename T>
    class Pool
    {
        // a storage entity
        struct Item
        {
            struct Comp
            {
                bool operator() (const  Item * lhs, const Item * rhs)
                {
                    return lhs->use_count < rhs->use_count;
                }
            };

            explicit Item(T && obj): use_count(0), object(std::move(obj)) {}

            uint64_t use_count;
            T        object;
        };

    public:
        explicit Pool(size_t maxsize): m_maxsize(maxsize), m_cursize(0) {}
        ~Pool() = default;

        // add an object to the pool
        int add(const T & obj)
        {
            if (m_cursize >= m_maxsize)
            {
                return -1;  // the pool is full
            }

            m_free.push_back(obj);
            m_cursize++;
            return 0;
        }

        // get a useable instance
        T get()
        {
            if (m_free.empty())
            {
                return T(); // no free space to allocate
            }

            auto obj = m_free.front();
            m_busy.push_back(obj);
            m_free.pop_back();
            return obj;
        }

        int free(const T & obj)
        {
            remove(obj);
            return add(obj);
        }

        void remove(const T & obj)
        {
            m_free.remove(obj);
            m_busy.remove(obj);
            m_cursize = m_free.size() + m_busy.size();
        }

    private:
        const size_t                    m_maxsize;
        size_t                          m_cursize;
        std::list<T>                    m_free;
        std::list<T>                    m_busy;
    };

}


#endif //TINYRPC_POOL_H
