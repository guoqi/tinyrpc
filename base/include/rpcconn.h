//
// Created by qiguo on 2/1/18.
//

#ifndef __TINYRPC_RPCCONN_H__
#define __TINYRPC_RPCCONN_H__

#include "conn.h"
#include "event.h"
#include "net.h"
#include "protocol.h"
#include <memory>

// rpc connection class
namespace tinyrpc
{
    class RpcConn : util::noncopyable
    {
        enum class AddrType {
            IPV4 = 1,
            UDS = 2
        };

    public:
        RpcConn(tinynet::EventLoop & loop, const tinynet::Ip4Addr & addr);
        RpcConn(tinynet::EventLoop & loop, const tinynet::UdsAddr & addr);

        // allow moveable
        RpcConn(RpcConn && rpcconn) noexcept;
        ~RpcConn() = default;

        RpcConn & send(const Message & msg);

        RpcConn & recv(const std::function<void(Message & msg)> & cb);

    private:
        void init();
        void connect();
        void reconnect();

    private:    // event handler
        void handleHeartBeat(tinynet::EventLoop & loop);
        void handleConnected(std::shared_ptr<tinynet::TcpConn> conn);

    private:
        std::shared_ptr<tinynet::TcpConn>   m_conn;
        AddrType                            m_addrType;
    };
}

#endif //TINYRPC_RPCCONN_H
