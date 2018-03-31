//
// Created by qiguo on 3/17/18.
//


#include <errlist.h>
#include "server.h"

using namespace std;
using namespace tinynet;

namespace tinyrpc
{
    void Server::handleService(const std::string &service, const Message &msg)
    {
        shared_ptr<TcpConn> client = TcpConn::createAttacher(m_loop, msg.clientfd());
        if (m_services.find(service) == m_services.end())
        {
            // TODO no such service. response to client and close connection
        }

        auto func = m_services[service];

        int ret = m_app->makeClient([func, client, msg](){
            Message retval;
            func(msg, retval);
            Server::clientResp(client, retval);
        });
    }

    void Server::bind(const std::string & service, ServiceFunc func)
    {
        m_services[service] = std::move(func);
    }

    void Server::clientResp(std::shared_ptr<tinynet::TcpConn> client, const Message &retval)
    {
        client->onWrite([=](shared_ptr<TcpConn> c){
            retval.sendBy(c);
            c->detach();
        });
        client->readwrite(false, true);
    }

    void ServerPool::add(std::shared_ptr<Server> server)
    {
        m_servers.push_back(server);
        auto idx = m_servers.size() - 1; // server's index
        for (auto & it : server->m_services)
        {
            m_services[m_count] = it.first;
            m_svc2svr[m_count] = idx;
            m_count++;
        }
    }

    std::pair< std::shared_ptr<Server>, std::string > ServerPool::locate(uint64_t sid)
    {
        if (m_services.find(sid) == m_services.end())
        {
            return std::pair();
        }

        if (m_svc2svr.find(sid) == m_svc2svr.end())
        {
            return std::pair();
        }

        return std::make_pair(m_servers[m_svc2svr.at(sid)], m_services.at(sid));
    }

    App::App(int max_client):
        m_loop(max_client), m_clients((size_t)max_client), m_max_client(max_client)
    {
    }

    void App::start()
    {
        m_main_thread = Thread::create([](){
            m_loop.start();
        });
    }

    void App::stop()
    {
        m_loop.stop();
    }

    int App::makeClient(const ThreadFunc & func)
    {
        return m_clients.add(Thread::create(func));
    }
}
