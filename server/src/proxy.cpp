//
// Created by qiguo on 1/22/18.
//

#include "proxy.h"
#include "server.h"
#include "thread.h"

using namespace std;
using namespace tinynet;
using namespace std::placeholders;

namespace tinyrpc
{
    Proxy::Proxy(const Config &config)
        : m_loop(config.proxy().maxConn()),
          m_client_pool((size_t)config.proxy().maxConn()),
          m_config(config)
    {
        m_proxy = TcpServer::startServer(m_loop, config.main().host(), config.main().port());
        m_proxy->onClientAccepted(std::bind(&Proxy::handleAccept, this, _1));
    }

    void Proxy::start()
    {
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

    void Proxy::dispatch(std::shared_ptr<TcpConn> & client, const Message & msg)
    {
        debug("%s", msg.data().c_str());

        switch (msg.protocol())
        {
            case HEARTBEAT: {
                Message retval;
                retval.protocol(HEARTBEAT);
                retval.seqno(msg.seqno() + 1);
                clientOk(client, retval);
                break;
            }
            case HANDSHAKE:
                // TODO
                break;
            case MESSAGE: {
                auto dst = ServerPool::instance().locate(msg.dst());
                if (! dst.first)
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
                client->close();
                break;
        }
    }

    void Proxy::handleAccept(std::shared_ptr<tinynet::TcpConn> client)
    {
        if (m_client_pool.add(client) == -1)
        {
            string errmsg = "connection pool is full";
            error("%s", errmsg.c_str());
            clientError(client, errmsg);
            return;
        }

        client->onRead([this](shared_ptr<TcpConn> c){
            Message msg = Message::recvBy(c);
            this->dispatch(c, msg);
        });
    }

    void Proxy::clientError(std::shared_ptr<tinynet::TcpConn> &client, const std::string & errmsg)
    {
        client->onWrite([errmsg](shared_ptr<TcpConn> c){
            Message msg;
            msg.data(errmsg);
            msg.sendBy(c);
            c->readwrite(true, false);
        });
        client->readwrite(false, true);
    }

    void Proxy::clientOk(std::shared_ptr<tinynet::TcpConn> & client, const Message &retval)
    {
        client->onWrite([retval](shared_ptr<TcpConn> c){
            retval.sendBy(c);
            c->readwrite(true, false);
        });
        client->readwrite(false, true);
    }
}
