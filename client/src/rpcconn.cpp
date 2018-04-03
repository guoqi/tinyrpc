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

const static int HEARTBEAT_TIME = 1000;  // send heartbeat packet every one second

namespace tinyrpc
{
    RpcConn::RpcConn(tinynet::EventLoop &loop, const tinynet::Ip4Addr &addr)
    {
        m_conn = TcpConn::createConnection(loop, addr);
        connect();
    }

    RpcConn::RpcConn(tinynet::EventLoop &loop, const tinynet::UdsAddr &addr)
    {
        m_conn = TcpConn::createConnection(loop, addr);
        connect();
    }

    std::shared_ptr<RpcConn> RpcConn::send(const Message &msg)
    {
        m_conn->onWrite([msg](shared_ptr<TcpConn> conn){
            msg.sendBy(conn);
            conn->readwrite(true, false);
        });

        return shared_from_this();
    }

    std::shared_ptr<RpcConn> RpcConn::recv(Message &retval)
    {
        bool readable = false;

        m_conn->onRead([&readable](shared_ptr<TcpConn> conn){
            readable = true;
        });

        // wait for connection readable
        while (! readable)
        {
            // TODO sleep
        }

        retval = Message::recvBy(m_conn);

        return shared_from_this();
    }

    std::shared_ptr<RpcConn> RpcConn::asyn_recv(const std::function<void(Message & msg)> & cb)
    {
        m_conn->onRead([cb](shared_ptr<TcpConn> conn){
            Message msg = Message::recvBy(conn);

            cb(msg);

            conn->readwrite(false, true);
        });

        return shared_from_this();
    }

    void RpcConn::init()
    {
        // add heartbeat event
        m_conn->loop().runAfter(0, std::bind(&RpcConn::handleHeartBeat, this, _1), HEARTBEAT_TIME);
    }

    void RpcConn::connect()
    {
        m_conn->onConnected(std::bind(&RpcConn::handleConnected, this, _1));
    }

    void RpcConn::reconnect()
    {
        shared_ptr<TcpConn> conn;
        switch (m_addrType)
        {
            case AddrType::IPV4:
                conn = TcpConn::createConnection(m_conn->loop(), m_conn->addr<Ip4Addr>());
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
    }

    Connector::Connector(int max_conn)
        : m_loop (max_conn)
    {
        m_main_thread = Thread::create([](){
            m_loop.start();
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

        std::pair<string, int> addr;
        if (m_rpcconn_pool.find(addr) == m_rpcconn_pool.end())
        {
            m_rpcconn_pool[addr] = make_shared<RpcConn> (m_loop, Ip4Addr(addr.first, addr.second));
        }

        return m_rpcconn_pool.at(addr);
    }

    void Connector::call(const std::string &service_uri, const Message &msg, Message &retval)
    {
        auto conn = get(service_uri);

        conn->send(msg)->recv(retval);
    }

    void Connector::asyn_call(const std::string &service_uri, const Message &msg, const MessageCallback &cb)
    {
        auto conn = get(service_uri);

        conn->send(msg)->asyn_recv(cb);
    }
}
