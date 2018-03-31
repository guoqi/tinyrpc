//
// Created by qiguo on 2/1/18.
//

#include "rpcconn.h"
#include "errlist.h"
#include "util.h"
#include <memory>

using namespace std;
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

    RpcConn::RpcConn(RpcConn &&rpcconn) noexcept
    {
        m_conn = std::move(rpcconn.m_conn);
    }

    RpcConn & RpcConn::send(const Message &msg)
    {
        m_conn->onWrite([](shared_ptr<TcpConn> conn){
            msg.sendBy(conn);
            conn->readwrite(true, false);
        });

        return *this;
    }

    RpcConn & RpcConn::recv(const std::function<void(Message & msg)> & cb)
    {
        m_conn->onRead([](shared_ptr<TcpConn> conn){
            Message msg = Message::recvBy(conn);

            cb(msg);

            conn->readwrite(false, true);
        });

        return *this;
    }

    void RpcConn::init()
    {
        // add heartbeat event
        m_conn->loop().runAfter(0, std::bind(RpcConn::handleHeartBeat, this), HEARTBEAT_TIME);
    }

    void RpcConn::connect()
    {
        m_conn->onConnected(std::bind(RpcConn::handleConnected, this));
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
            send(Message(HEARTBEAT)).recv([](Message & msg){
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
}
