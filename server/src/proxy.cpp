//
// Created by qiguo on 1/22/18.
//

#include "proxy.h"
#include "server.h"

using namespace std;
using namespace tinynet;
using namespace std::placeholders;

namespace tinyrpc
{
    Proxy::Proxy(const Config &config)
        : m_loop(config.proxy().maxConn()),
          m_maxclient((size_t)config.proxy().maxConn()),
          m_config(config)
    {
        m_proxy = TcpServer::startServer(m_loop, config.main().host(), config.main().port());
        m_proxy->onClientAccepted(std::bind(&Proxy::handleAccept, this, _1));

        setHeartbeat();
    }

    void Proxy::start()
    {
        // start server
        for (const auto & server : ServerPool::instance().servers())
        {
            int threads = 0;
            try {
                threads = m_config.server(server->name()).threads();
            } catch (std::out_of_range & e) {
                threads = m_config.proxy().threads();
            }

            server->initApp(threads);
        }

        // start self
        m_loop.start();
    }

    void Proxy::stop()
    {
        for (const auto & server : ServerPool::instance().servers())
        {
            server->stopApp();
        }

        m_loop.stop();
    }

    void Proxy::dispatch(std::shared_ptr<TcpConn> client, const Message & msg)
    {
        switch (msg.protocol())
        {
            case HEARTBEAT:
                m_clients[client->fd()].second = util::Time::nowMs();
                debug("recv heartbeat");
                break;
            case HANDSHAKE:
                // TODO
                break;
            case MESSAGE: {
                auto dst = ServerPool::instance().locate(msg.dst());
                if (dst.first != nullptr)
                {
                    dst.first->handleService(dst.second, msg);
                }
                else
                {
                    clientError(client, "not found destination service");
                }
                break;
            }
            default:
                clientError(client, "invalid message protocol");
                break;
        }
    }

    void Proxy::handleAccept(std::shared_ptr<tinynet::TcpConn> client)
    {
        if (m_clients.size() >= m_maxclient)
        {
            string errmsg = "connection pool is full";
            error("%s", errmsg.c_str());
            clientError(client, errmsg);
            return;
        }

        //m_clients[client->fd()] = std::make_pair(client, util::Time::now());
        m_clients.insert(std::make_pair(client->fd(), std::make_pair(client, util::Time::nowMs())));

        client->onRead([this](shared_ptr<TcpConn> c){
            bool stop = false;
            while (! stop)  // avoid sticky package
            {
                try
                {
                    Message msg = Message::recvBy(c);
                    msg.clientfd(c->fd());
                    this->dispatch(c, msg);
                }
                catch (util::TinyExp & e)
                {
                    if (e.code() == NET_PEER_CLOSE)
                    {
                        m_clients.erase(c->fd());
                    }
                    else if (e.code() != NET_WOULD_BLOCK)
                    {
                        info("[%d:%s] unexpected error happened when read from client.", e.code(), e.what());
                    }
                    stop = true;
                }
            }
        });
    }

    void Proxy::clientError(std::shared_ptr<tinynet::TcpConn> client, const std::string & errmsg)
    {
        client->onWrite([errmsg](shared_ptr<TcpConn> c){
            Message msg;
            msg.data(errmsg);
            msg.sendBy(c);
            c->readwrite(true, false);
        });
        client->readwrite(false, true);
    }

    void Proxy::clientOk(std::shared_ptr<tinynet::TcpConn> client, const Message &retval)
    {
        client->onWrite([retval](shared_ptr<TcpConn> c){
            retval.sendBy(c);
            c->readwrite(true, false);
        });
        client->readwrite(false, true);
    }

    void Proxy::setHeartbeat()
    {
        m_loop.runAfter(0, [this](EventLoop & loop){
            for (const auto & item : m_clients) {
                Message msg(HEARTBEAT);

                try
                {
                    msg.sendBy(item.second.first);
                }
                catch (util::TinyExp & e)
                {
                    if (e.code() != NET_WOULD_BLOCK)
                    {
                        m_clients.erase(item.first);
                    }
                }
            }
        }, HBINTVAL);

        m_loop.runAfter(3 * HBINTVAL, [this](EventLoop & loop){
            for (const auto & item : m_clients) {
                if (util::Time::nowMs() - item.second.second >= 3 * HBINTVAL)
                {
                    m_clients.erase(item.first);
                }
            }
        }, 3 * HBINTVAL);
    }
}
