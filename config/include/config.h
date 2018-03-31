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

    private:
        std::string         m_host;
        int                 m_port;
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

        inline int threads() const { return m_threads; }
        inline int server_default_threads() const { return m_server_default_threads; }
        inline int maxConn() const { return m_maxConn; }

    private:
        int             m_threads;
        int             m_maxConn;  // max connection num
        int             m_server_default_threads;
    };

    class ServerSection
    {
    public:
        explicit ServerSection(std::string servername): m_servername(std::move(servername)), m_threads(-1) {}
        ~ServerSection() = default;

        std::string servername() const { return m_servername; }
        int threads() const { return m_threads; }

    private:
        std::string             m_servername;
        int                     m_threads;
    };

    /**
     * Config consists of a main section and some server sections.
     * For example, here is a simple config format:
     * {
     *    "main": {
     *        "listen": <[host:]port>,    # host is 0.0.0.0 by default
     *    },
     *
     *    "master": {
     *    },
     *
     *    "proxy": {
     *        "threads":  <num>
     *        "max_connection": <num>
     *        "server_default_threads": <num>
     *    },
     *
     *    "server_1": {
     *        "threads": <num>
     *    },
     *
     *    "server_2": {
     *        "threads": <num>
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
