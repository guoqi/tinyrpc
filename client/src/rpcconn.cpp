//
// Created by qiguo on 2/1/18.
//

#include "rpcconn.h"
#include "errlist.h"
#include "util.h"
#include <memory>

using namespace std;
using namespace std::placeholders;
using namespace tinynet;
using namespace util;

const static int HEARTBEAT_TIME = 1000;  // send heartbeat packet every one second

namespace tinyrpc
{
    RpcConn::RpcConn(tinynet::EventLoop &loop, const tinynet::Ip4Addr &addr)
        : m_addrType(AddrType::IPV4)
    {
        m_conn = TcpConn::createConnection(loop, addr);
        connect();
    }

    RpcConn::RpcConn(tinynet::EventLoop &loop, const tinynet::UdsAddr &addr)
        : m_addrType(AddrType::UDS)
    {
        m_conn = TcpConn::createConnection(loop, addr);
        connect();
    }

    RpcConn * RpcConn::send(const Message &msg)
    {
        m_conn->onWrite([msg](shared_ptr<TcpConn> conn){
            msg.sendBy(conn);
            conn->readwrite(true, false);
        });

        return this;
    }

    RpcConn * RpcConn::recv(Message &retval)
    {
        m_conn->onRead([this, &retval](shared_ptr<TcpConn> conn){
            debug("fd=%d, %p", conn->fd(), conn.get());
            debug("%p", this);
            try
            {
                debug("%p", this);
                retval = Message::recvBy(conn);
            }
            catch (util::TinyExp & e)
            {
                debug("fd=%d, %p", conn->fd(), conn.get());
                debug("fd=%d, %p", m_conn->fd(), m_conn.get());
                debug("%p", this);
                reconnect();
            }
            debug("%p", this);
        });

        debug("%p", m_conn.get());
        debug("%p", this);

        // wait for connection readable
        while (m_conn->state() != ConnState::READ && m_conn->state() != ConnState::FAIL)
        {
            // TODO sleep to avoid cpu always running
        }

        return this;
    }

    RpcConn* RpcConn::asyn_send(const Message &msg)
    {
        m_asyn_send_queue.emplace_back([this](Message & msg) {
            msg.sendBy(m_conn);
        });

        return this;
    }

    RpcConn * RpcConn::asyn_recv(const std::function<void(Message & msg)> & cb)
    {
        m_conn->onRead([this, cb](shared_ptr<TcpConn> conn){
            Message msg;
            try
            {
                msg = Message::recvBy(conn);

                cb(msg);
            }
            catch (util::TinyExp & e)
            {
                reconnect();
            }
        });

        return this;
    }

    void RpcConn::init()
    {
        // add heartbeat event
        // m_conn->loop().runAfter(0, std::bind(&RpcConn::handleHeartBeat, this, _1), HEARTBEAT_TIME);
    }

    void RpcConn::connect()
    {
        m_conn->onConnected(std::bind(&RpcConn::handleConnected, this, _1));

        // TODO pthread wait
        // wait for connection establishing
        while (m_conn->state() != ConnState::CONN && m_conn->state() != ConnState::FAIL) {}
    }

    void RpcConn::reconnect()
    {
        debug("fd=%d, %p", m_conn->fd(), m_conn.get());
        m_conn->close();

        debug("hhhh");

        shared_ptr<TcpConn> conn;
        switch (m_addrType)
        {
            case AddrType::IPV4:
                debug("hhhh");
                conn = TcpConn::createConnection(m_conn->loop(), m_conn->addr<Ip4Addr>());
                debug("hhhh");
                break;
            case AddrType::UDS:
                conn = TcpConn::createConnection(m_conn->loop(), m_conn->addr<UdsAddr>());
                break;
        }
        m_conn = conn;
        connect();
    }

    void RpcConn::handleHeartBeat(tinynet::EventLoop &loop)
    {
        try
        {
            send(Message(HEARTBEAT))->asyn_recv([](Message & msg){
                panicif(msg.protocol() == HEARTBEAT, ERR_INVALID_MESSAGE, "received not heart packet");
            });
        }
        catch (util::SysExp & e)
        {
            error("something trouble with network. prepare to reconnect.");
            reconnect();
        }
    }

    void RpcConn::handleConnected(std::shared_ptr<tinynet::TcpConn> conn)
    {
        init();

        // TODO singal a pthread cond variable
    }

    Connector::Connector(int max_conn)
        : m_loop (max_conn)
    {
        m_main_thread = Thread::create([this](){
            try
            {
                m_loop.start();
            }
            catch (TinyExp & e)
            {
                info("%s", e.what());
                m_loop.stop();
            }
            catch (std::exception & e)
            {
                info("%s", e.what());
                m_loop.stop();
            }
        });
    }

    Connector& Connector::instance()
    {
        static Connector connector(100000);
        return connector;
    }

    RpcConn::Ptr Connector::get(const std::string &service_uri)
    {
        // TODO split service_url and accquire coresponding (ip, port) tuple

        std::pair<string, int> addr = {"127.0.0.1", 7777};
        debug("%d", m_rpcconn_pool.find(addr) == m_rpcconn_pool.end());
        if (m_rpcconn_pool.find(addr) == m_rpcconn_pool.end())
        {
            m_rpcconn_pool[addr] = make_shared<RpcConn> (m_loop, Ip4Addr(addr.first, addr.second));
        }

        return m_rpcconn_pool.at(addr);
    }

    void Connector::call(const std::string &service_uri, const Message &msg, Message &retval)
    {
        auto conn = get(service_uri);

        if (! conn->fail())
        {
            conn->send(msg)->recv(retval);
            debug("%p", conn.get());
        }
    }

    void Connector::asyn_call(const std::string &service_uri, const Message &msg, const MessageCallback &cb)
    {
        auto conn = get(service_uri);

        if (! conn->fail())
        {
            conn->send(msg)->asyn_recv(cb);
        }
    }
}
