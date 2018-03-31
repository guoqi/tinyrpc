//
// Created by qiguo on 1/22/18.
//

#include "proxy.h"
#include "server.h"
#include "thread.h"

using namespace std;
using namespace tinynet;

namespace tinyrpc
{
    Proxy::Proxy(const Config &config, int fd)
        : m_loop(config.proxy().maxConn()),
          m_client_pool(config.proxy().maxConn())
    {
        m_proxy = TcpConn::createAttacher(m_loop, fd);
        m_proxy->onRead(std::bind(Proxy::handleAccept, this));
        m_proxy->readwrite(true, false);
    }

    void Proxy::dispatch(std::shared_ptr<TcpConn> & client, const Message & msg)
    {
        switch (msg.protocol())
        {
            case HEARTBEAT:
                Message retval;
                retval.protocol(HEARTBEAT);
                retval.seqno(msg.seqno() + 1);
                clientOk(client, retval);
                break;
            case HANDSHAKE:
                // TODO
                break;
            case MESSAGE:
                auto dst = ServerPool::locate(msg.dst());
                dst.first->handleService(dst.second, msg);
                break;
            default:
                client->close();
                break;
        }
    }

    void Proxy::handleAccept(std::shared_ptr<tinynet::TcpConn> conn)
    {
        int cfd;
        conn->recv((char *)&cfd, sizeof(cfd)); // read client fd

        handleClient(cfd);
    }

    void Proxy::handleClient(int cfd)
    {
        auto client = TcpConn::createAttacher(m_loop, cfd);

        if (m_client_pool.add(client) == -1)
        {
            string errmsg = "connection pool is full";
            error("%s", errmsg.c_str());
            clientError(client, errmsg);
            return;
        }

        client->onRead([](shared_ptr<TcpConn> c){
            Message msg = Message::recvBy(client);
            dispatch(c, msg);
        });
    }

    void Proxy::clientError(std::shared_ptr<tinynet::TcpConn> &client, const std::string & errmsg)
    {
        client->onWrite([errmsg](shared_ptr<TcpConn> c){
            Message msg;
            msg.data(errmsg);
            msg.sendBy(c);
            c->close();
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
