//
// Created by qiguo on 1/21/18.
//

#ifndef __TINYRPC_CONFIG_H__
#define __TINYRPC_CONFIG_H__

#include <string>
#include <vector>
#include <map>

namespace tinyrpc
{
    class Config;
    class MainSection;
    class MasterSection;
    class ProxySection;
    class ServerSection;

    class MainSection
    {
    public:
        MainSection() = default;
        ~MainSection() = default;

        inline const std::string & host() const { return m_host; }
        inline int port() const { return m_port; }
        inline int maxConn() const { return m_maxConn; }

    private:
        std::string         m_host;
        int                 m_port;
        int                 m_maxConn;  // max connection num
    };

    class MasterSection
    {
        // TODO
    };

    class ProxySection
    {
    public:
        ProxySection() = default;
        ~ProxySection() = default;

        const std::vector<std::string> & servers() const { return m_servers; }

    private:
        std::vector<std::string> m_servers;
    };

    class ServerSection
    {
    public:
        explicit ServerSection(const std::string & servername): m_servername(servername) {}
        ~ServerSection() = default;

        std::string servername() const { return m_servername; }
        std::string udsname() const { return m_udsname; }

    private:
        std::string             m_servername;
        std::string             m_udsname;
    };

    /**
     * Config consists of a main section and some server sections.
     * For example, here is a simple config format:
     * {
     *    "main": {
     *        "listen": <[host:]port>,    # host is 0.0.0.0 by default
     *        "max_connection": <num>
     *    },
     *
     *    "master": {
     *    },
     *
     *    "proxy": {
     *        "servers": [server_name list]
     *    },
     *
     *    "server_1": {
     *        "uds_name": <unix domain socket name>
     *    },
     *
     *    "server_2": {
     *        "uds_name": <unix domain socket name>
     *    }
     * }
     */
    class Config
    {
    public:
        Config(const std::string filename);
        ~Config() = default;

        const MainSection & main() const { return m_main; }
        const MasterSection & master() const { return m_master; }
        const ProxySection & proxy() const { return m_proxy; }
        const ServerSection & server(const std::string servername) const { return m_serevers.at(servername); }

    private:
        MainSection                             m_main;
        MasterSection                           m_master;
        ProxySection                            m_proxy;
        std::map<std::string, ServerSection>    m_serevers;
    };

}

#endif //RPCSERVER_CONFIG_H
