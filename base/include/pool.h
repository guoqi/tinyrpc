//
// Created by qiguo on 2/22/18.
//

#ifndef __TINYRPC_POOL_H__
#define __TINYRPC_POOL_H__

#include <cstdint>
#include <list>
#include <set>
#include <memory>

namespace tinyrpc
{

    template<typename T>
    class Pool;


    template<typename T>
    class Item
    {
    public:
        typedef std::shared_ptr< Item<T> >  Ptr;

        Item(Pool<T> & pool, std::shared_ptr<T> data) : m_pool(pool), m_data(data), m_using(false) {}
        Item(const Item<T> & item): m_pool(item.m_pool), m_data(item.m_data), m_using(item.m_using) {}

        ~Item() = default;

        void free() { m_using = false; }
        void use() { m_using = true; }
        bool using_status() { return m_using; }

        std::shared_ptr<T> data() const { return m_data; }

    private:
        Pool<T>   &                      m_pool;
        std::shared_ptr<T>               m_data;
        bool                             m_using;
    };

    /**
     * Base common pool for any presist instance.
     * The instance must have copy constructor and move constructor and copyable
     */
    template<typename T>
    class Pool
    {
    public:
        explicit Pool(size_t maxsize): m_maxsize(maxsize), m_cursize(0) {}
        ~Pool() = default;

        // requrire type T has a constructor without any parameters
        void init(size_t size)
        {
            for (size_t i=0; i<size; ++i)
            {
                add(std::make_shared<T> ());
            }
        }

        void init(const std::list< std::shared_ptr<T> > & vdata)
        {
            for (auto const & item : vdata)
            {
                add(item);
            }
        }

        int add(std::shared_ptr<T> data)
        {
            return add(std::make_shared< Item<T> > (*this, data));
        }

        // add an object to the pool
        int add(typename Item<T>::Ptr obj)
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
        typename Item<T>::Ptr get()
        {
            if (m_free.empty())
            {
                return nullptr; // no free space to allocate
            }

            auto obj = m_free.front();
            obj->use();
            m_free.pop_front();
            m_busy.push_back(obj);
            return obj;
        }

        int free(typename Item<T>::Ptr obj)
        {
            if (! obj->using_status()) {
                return 0;
            }

            m_busy.remove(obj);
            obj->free();
            m_free.push_front(obj);
        }

        void remove(typename Item<T>::Ptr obj)
        {
            if (obj->using_status())
            {
                m_busy.remove(obj);
            }
            else
            {
                m_free.remove(obj);
            }

            m_cursize--;
        }

        const std::list< typename Item<T>::Ptr > & pool() const { return m_busy; }

    private:
        const size_t                             m_maxsize;
        size_t                                   m_cursize;
        std::list< typename Item<T>::Ptr >       m_free;
        std::list< typename Item<T>::Ptr >       m_busy;
    };

}


#endif //TINYRPC_POOL_H
