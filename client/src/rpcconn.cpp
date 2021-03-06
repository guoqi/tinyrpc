//
// Created by qiguo on 2/1/18.
//

#include "rpcconn.h"
#include "errlist.h"
#include "util.h"
#include <memory>
#include <config.h>

using namespace std;
using namespace std::placeholders;
using namespace tinynet;
using namespace util;


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
        m_asyn_send_queue.push_back([this, msg](){
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

        m_asyn_recv_queue.push_back(std::pair<RecvCallback, ErrorCallback>(cb, errhandler));

        m_cond.wait();

        return this;
    }

    RpcConn* RpcConn::asyn_send(const Message &msg, const ErrorCallback & errhandler)
    {
        m_asyn_send_queue.push_back([this, msg, errhandler]() {
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
        m_asyn_recv_queue.push_back(std::pair<RecvCallback, ErrorCallback>(cb, errhandler));

        return this;
    }

    void RpcConn::handleRead(const std::shared_ptr<tinynet::TcpConn> &conn)
    {
        if (m_should_reconnect)
        {
            reconnect();
        }

        do
        {
            RecvCallback recvcb = [](Message & e) {};
            ErrorCallback errhandler = [](const TinyExp & e) {};

            if (! m_asyn_recv_queue.empty()) {
                auto &item = m_asyn_recv_queue.front();
                recvcb = item.first;
                errhandler = item.second;
            }

            try
            {
                Message msg = Message::recvBy(conn);

                switch(msg.protocol())
                {
                    case HEARTBEAT: {
                        Message retval(HEARTBEAT);
                        asyn_send(retval, [](const TinyExp & e){});
                        m_last_heartbeat = Time::nowMs();
                        debug("recv heartbeat");
                        break;
                    }
                    case MESSAGE:
                        recvcb(msg);
                        if (! m_asyn_recv_queue.empty()) {
                            m_asyn_recv_queue.pop_front();  // pop after assignment
                        }
                        break;
                    default: {
                        Message retval;
                        retval.data("not valid message protocol");
                        asyn_send(retval, [](const TinyExp & e){});
                        break;
                    }
                }
            }
            catch (const util::TinyExp & e)
            {
                if (e.code() == NET_WOULD_BLOCK)
                {
                    break;
                }

                // else
                errhandler(e);

                if (! m_asyn_recv_queue.empty()) {
                    m_asyn_recv_queue.pop_front();
                }
            }
            catch (...)
            {
                throw;
            }
        } while (! m_asyn_recv_queue.empty());
        conn->readwrite(true, true);
    }

    void RpcConn::handleWrite(const std::shared_ptr<tinynet::TcpConn> &conn)
    {
        if (m_should_reconnect)
        {
            reconnect();
        }

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
        m_should_reconnect = false;
        m_last_heartbeat = 0;

        m_conn->onRead(std::bind(&RpcConn::handleRead, this, _1));
        m_conn->onWrite(std::bind(&RpcConn::handleWrite, this, _1));

        // heartbeat check
        m_conn->loop().runAfter(HBINTVAL * 3, [this](EventLoop & loop){
            if (Time::nowMs() - m_last_heartbeat > HBINTVAL * 3) {
                m_should_reconnect = true;
            }
        }, HBINTVAL * 3);
    }

    void RpcConn::connect()
    {
        m_conn->onConnected([this](std::shared_ptr<TcpConn> conn){
            init();
            m_cond.signal();
        });

        m_cond.wait();
    }

    void RpcConn::asyn_connect()
    {
        m_conn->onConnected([this](std::shared_ptr<TcpConn> conn){
            init();
        });
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
                m_conn = TcpConn::createConnection(m_conn->loop(), m_conn->addr<Ip4Addr>());
                debug("hhhh");
                break;
            case AddrType::UDS:
                m_conn = TcpConn::createConnection(m_conn->loop(), m_conn->addr<UdsAddr>());
                break;
        }
        asyn_connect();
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
