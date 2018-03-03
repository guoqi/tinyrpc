//
// Created by qiguo on 1/22/18.
//

#include "proxy.h"

using namespace std;
using namespace tinynet;

namespace tinyrpc
{
    Proxy::Proxy(const Config &config)
        : m_config(config), m_loop(m_config.main().maxConn())
    {
        m_proxy = TcpServer::startServer(m_loop, m_config.main().host(), m_config.main().port());

        init(config);
    }

    void Proxy::reload(const Config &config)
    {
        init(config);
    }

    void Proxy::dispatch(const std::string servername, int fd)
    {
        // TODO
    }

    void Proxy::init(const Config &config)
    {
        for (auto const & server : m_config.proxy().servers())
        {
            auto rpcconn = make_shared<RpcConn>(m_loop, UdsAddr(m_config.server(server).udsname()));
            m_servers.push_back(rpcconn);
        }
    }
}
