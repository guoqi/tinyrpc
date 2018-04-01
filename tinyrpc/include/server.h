//
// Created by qiguo on 3/17/18.
//

#ifndef __TINYRPC_SERVER_H__
#define __TINYRPC_SERVER_H__

#include "util.h"
#include "protocol.h"
#include "event.h"
#include "thread.h"
#include "pool.h"
#include <string>
#include <functional>
#include <map>
#include <set>
#include <memory>
#include <vector>

#define DECLARE_SERVER_CLASS(server) \
    ServerContext<server> __context##server

namespace tinyrpc
{
    using ServiceFunc = std::function<void(const Message & msg, Message & retval)>;

    class App;

    class Server: public util::noncopyable, public std::enable_shared_from_this
    {
    public:
        friend class ServerPool;
        Server() = default;
        virtual ~Server() = default;

        virtual void initialize() = 0;
        virtual void destory() = 0;

        void initApp(int max_client) { m_app = std::make_shared<App>(max_client); m_app->start(); }
        void stopApp() { m_app->stop(); }

        std::string name() const { return m_name; }

        void handleService(const std::string & service, const Message & msg);

    protected:
        void bind(const std::string & service, ServiceFunc);
        static void clientResp(std::shared_ptr<tinynet::TcpConn> client, const Message & retval);

    private:
        const std::string                   m_name;
        std::map<std::string, ServiceFunc>  m_services;
        std::shared_ptr<App>                m_app;
    };

    class ServerPool: public util::noncopyable
    {
    public:
        ~ServerPool() = default;

        static void add(std::shared_ptr<Server> server);

        static std::pair< std::shared_ptr<Server>, std::string > locate(uint64_t sid);

        static const std::vector< std::shared_ptr<Server> > servers() { return m_servers; }

    private:
        ServerPool() = default;

    private:
        static std::map< uint64_t, std::string >               m_services;
        static std::map< uint64_t, size_t >                    m_svc2svr;
        static std::vector< std::shared_ptr<Server> >          m_servers;
        static uint64_t                                        m_count;
    };

    // manage server context
    template<typename T>
    class ServerContext
    {
    public:
        ServerContext()
        {
            m_server = std::make_shared<T>();
            ServerPool::add(m_server);
            m_server->initialize();
        }

        virtual ~ServerContext()
        {
            m_server->destory();
        }

    private:
        std::shared_ptr<T>  m_server;
    };

    class App: public util::noncopyable
    {
    public:
        App(int max_client);
        virtual ~App() {}

        void start();
        void stop();
        int makeClient(const ThreadFunc & func);

    protected:
        int                                     m_max_client;
        tinynet::EventLoop                      m_loop;
        std::shared_ptr<Thread>                 m_main_thread;
        Pool< std::shared_ptr<Thread> >         m_clients;
    };

}

#endif //TINYRPC_SERVER_H
