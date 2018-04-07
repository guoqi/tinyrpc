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
        m_asyn_send_queue.emplace_back([this, msg](){
            try
            {
                msg.sendBy(m_conn);
            }
            catch (const util::TinyExp & e)
            {

            }

            m_cond.signal();
        });

        m_cond.wait();

        return this;
    }

    RpcConn * RpcConn::recv(Message &retval)
    {
        auto cb = [this, &retval](Message & msg){
            retval = msg;
            m_cond.signal();
        };

        auto errhandler = [this](const TinyExp & e){
            m_cond.signal();
        };

        m_asyn_recv_queue.emplace_back(std::pair<RecvCallback, ErrorCallback>(cb, errhandler));

        m_cond.wait();

        return this;
    }

    RpcConn* RpcConn::asyn_send(const Message &msg, const ErrorCallback & errhandler)
    {
        m_asyn_send_queue.emplace_back([this, msg, errhandler]() {
            try
            {
                msg.sendBy(m_conn);
            }
            catch (const TinyExp & e)
            {
                errhandler(e);
            }
        });

        return this;
    }

    RpcConn * RpcConn::asyn_recv(const RecvCallback & cb, const ErrorCallback & errhandler)
    {
        m_asyn_recv_queue.emplace_back(std::pair<RecvCallback, ErrorCallback>(cb, errhandler));

        return this;
    }

    void RpcConn::handleRead(const std::shared_ptr<tinynet::TcpConn> &conn)
    {
        // info("%s, %d", __FUNCTION__, m_asyn_recv_queue.size());

        /*
        if (detectConn())
        {
            reconnect();
            return;
        }
        */

        while (! m_asyn_recv_queue.empty())
        {
            auto & item = m_asyn_recv_queue.front();
            try
            {
                Message msg = Message::recvBy(conn);

                (item.first)(msg);
            }
            catch (const util::TinyExp & e)
            {
                (item.second)(e);
            }
            catch (...)
            {
                m_asyn_recv_queue.pop_front();
                throw;
            }
            m_asyn_recv_queue.pop_front();
        }
        conn->readwrite(true, true);
    }

    void RpcConn::handleWrite(const std::shared_ptr<tinynet::TcpConn> &conn)
    {
        // info("%s, %d", __FUNCTION__, m_asyn_send_queue.size());

        /*
        if(detectConn())
        {
            reconnect();
        }
        */

        while (! m_asyn_send_queue.empty())
        {
            auto & item = m_asyn_send_queue.front();
            try
            {
                item();
            }
            catch (...)
            {
                m_asyn_send_queue.pop_front();
                throw;
            }
            m_asyn_send_queue.pop_front();
        }
        conn->readwrite(true, true);
    }

    void RpcConn::init()
    {
        // add heartbeat event
        // m_conn->loop().runAfter(0, std::bind(&RpcConn::handleHeartBeat, this, _1), HEARTBEAT_TIME);
        m_conn->onRead(std::bind(&RpcConn::handleRead, this, _1));
        m_conn->onWrite(std::bind(&RpcConn::handleWrite, this, _1));
    }

    void RpcConn::connect()
    {
        m_conn->onConnected(std::bind(&RpcConn::handleConnected, this, _1));

        m_cond.wait();
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

    bool RpcConn::detectConn()
    {
        bool ret;
        try
        {
            ret = m_conn->checkClosed();
        }
        catch (const util::TinyExp & e)
        {
            return false;
        }

        return ret;
    }

    /*
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
    */

    void RpcConn::handleConnected(std::shared_ptr<tinynet::TcpConn> conn)
    {
        init();

        conn->readwrite(false, true);

        m_cond.signal();
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
        }
    }

    void Connector::asyn_call(const std::string &service_uri, const Message &msg, const RecvCallback & cb, const ErrorCallback & errhandler)
    {
        auto conn = get(service_uri);

        if (! conn->fail())
        {
            conn->asyn_send(msg, errhandler)->asyn_recv(cb, errhandler);
        }
    }
}
