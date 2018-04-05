//
// Created by qiguo on 2/1/18.
//

#ifndef __TINYRPC_RPCCONN_H__
#define __TINYRPC_RPCCONN_H__

#include "conn.h"
#include "event.h"
#include "net.h"
#include "protocol.h"
#include "thread.h"
#include <memory>
#include <map>

// rpc connection class
namespace tinyrpc
{
    using MessageCallback = std::function<void(Message & msg)>;

    class RpcConn : util::noncopyable, std::enable_shared_from_this<RpcConn>
    {
        enum class AddrType {
            IPV4 = 1,
            UDS = 2
        };

    public:
        using Ptr = std::shared_ptr<RpcConn>;

        RpcConn(tinynet::EventLoop & loop, const tinynet::Ip4Addr & addr);
        RpcConn(tinynet::EventLoop & loop, const tinynet::UdsAddr & addr);

        ~RpcConn() = default;

        std::shared_ptr<RpcConn> send(const Message & msg);

        std::shared_ptr<RpcConn> recv(Message & retval);

        std::shared_ptr<RpcConn> asyn_recv(const MessageCallback & cb);

        bool fail() const { return m_conn->state() == tinynet::ConnState::FAIL; }

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

    class Connector : util::noncopyable
    {
    public:
        virtual ~Connector() { m_loop.stop(); }

        static Connector & instance();

        /*
         * synchronus remote process call
         *
         * @parma service_url       dot-seperated uri, eg: Hello.hello means call hello service which belongs to Hello server
         *        msg               request msg
         *        retval            response msg
         */
        void call(const std::string & service_uri, const Message & msg, Message & retval);

        /*
         * asynchronus remote process call
         * parameeter is simliar as call
         */
        void asyn_call(const std::string & service_uri, const Message & msg, const MessageCallback & cb);

    private:
        explicit Connector(int max_conn);

        RpcConn::Ptr get(const std::string & service_uri);

    private:
        std::map< std::pair<std::string, int >,
                RpcConn::Ptr >                      m_rpcconn_pool;
        tinynet::EventLoop                          m_loop;
        std::shared_ptr<Thread>                     m_main_thread;
    };
}

#endif //TINYRPC_RPCCONN_H
