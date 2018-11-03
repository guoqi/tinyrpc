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
#include "ts_queue.h"
#include <memory>
#include <map>
#include <list>

// rpc connection class
namespace tinyrpc
{
    using SendCallback = std::function<void()>;
    using RecvCallback = std::function<void(Message & msg)>;
    using ErrorCallback = std::function<void(const util::TinyExp & err)>;

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

        ~RpcConn() { m_conn->close(); debug("desctructor %p", this); }

        // syn operation
        RpcConn * send(const Message & msg);
        RpcConn * recv(Message & retval);

        // asyn operation
        RpcConn * asyn_send(const Message & msg, const ErrorCallback & errhandler);
        RpcConn * asyn_recv(const RecvCallback & cb, const ErrorCallback & errhandler);

        bool fail() const { return m_conn->state() == tinynet::ConnState::FAIL; }

    private:
        void init();
        void reconnect();
        void connect();
        void asyn_connect();

        void handleRead(const std::shared_ptr<tinynet::TcpConn> & conn);
        void handleWrite(const std::shared_ptr<tinynet::TcpConn> & conn);

    private:
        std::shared_ptr<tinynet::TcpConn>   m_conn;
        AddrType                            m_addrType;
        TSLockQueue<SendCallback>           m_asyn_send_queue;
        TSLockQueue< std::pair<RecvCallback,
                ErrorCallback> >            m_asyn_recv_queue;
        ThreadCond                          m_cond;
        bool                                m_should_reconnect;
        int64_t                             m_last_heartbeat;
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
        void asyn_call(const std::string & service_uri, const Message & msg, const RecvCallback & cb, const ErrorCallback & errhandler);

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
