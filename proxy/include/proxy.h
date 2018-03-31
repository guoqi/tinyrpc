//
// Created by qiguo on 1/21/18.
//

#ifndef __TINYRPC_PROXY_H__
#define __TINYRPC_PROXY_H__

#include "util.h"
#include "config.h"
#include "event.h"
#include "conn.h"
#include "rpcconn.h"
#include "pool.h"
#include <vector>

namespace tinyrpc
{
    class Proxy : util::noncopyable
    {
    public:
        Proxy(const Config & config, int fd);

        void start() { m_loop.start(); }
        void stop() { m_loop.stop(); }

        // dispatch request to a specific server process
        void dispatch(std::shared_ptr<tinynet::TcpConn> & client, const Message & msg);

    protected:
        void handleAccept(std::shared_ptr<tinynet::TcpConn> conn);

        void handleClient(int cfd);

        void clientError(std::shared_ptr<tinynet::TcpConn> & client, const std::string & errmsg);
        void clientOk(std::shared_ptr<tinynet::TcpConn> & client, const Message & retval);

    private:
        tinynet::EventLoop                      m_loop;
        std::shared_ptr<tinynet::TcpConn>       m_proxy;
        Pool< std::shared_ptr<
                tinynet::TcpConn> >             m_client_pool;
    };
}

#endif //TINYRPC_RPCPROXY_H
