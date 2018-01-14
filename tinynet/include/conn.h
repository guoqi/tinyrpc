//
// Created by qiguo on 1/4/18.
//

#ifndef __TINYRPC_CONN_H__
#define __TINYRPC_CONN_H__

#include <string>
#include <memory>
#include <unistd.h>
#include "util.h"
#include "event.h"
#include "net.h"

namespace tinynet
{
    class TcpConn;

    /**
     * Socket event type
     */
    enum class ConnState
    {
        INIT      = 0x01,
        CONN      = 0x02,
        READ      = 0x03,
        WRITE     = 0x04,
    };


    using TcpConnCallback = std::function<void(std::shared_ptr<TcpConn> conn)>;


    class BaseConn : public util::noncopyable
    {
    public:
        BaseConn() = default;
        virtual ~BaseConn() = default;
    };

    class TcpConn : public std::enable_shared_from_this<TcpConn>, public BaseConn
    {
    public:
        static std::shared_ptr<TcpConn> createConnection(EventLoop & loop, const std::string ip, int port);
        static std::shared_ptr<TcpConn> createConnection(EventLoop & loop, const Ip4Addr & addr);
        static std::shared_ptr<TcpConn> createAttacher(EventLoop & loop, int fd);

        explicit TcpConn(EventLoop & loop);
        TcpConn(EventLoop & loop, const std::string & ip, int port);
        TcpConn(EventLoop & loop, const Ip4Addr & addr);
        virtual ~TcpConn() = default;

        void closeRead() { m_event.readable(false); m_loop.alter(m_event); ::shutdown(m_event.fd(), SHUT_RD); }
        void closeWrite() { m_event.writeable(false); m_loop.alter(m_event); ::shutdown(m_event.fd(), SHUT_WR); }
        void close() { m_loop.remove(m_event); ::close(m_event.fd()); }

        void readwrite(bool read, bool write) { m_event.readable(read); m_event.writeable(write); m_loop.alter(m_event); }

        std::shared_ptr<TcpConn> onRead(const TcpConnCallback & cb);
        std::shared_ptr<TcpConn> onWrite(const TcpConnCallback & cb);
        std::shared_ptr<TcpConn> onConnected(const TcpConnCallback & cb);

        Ip4Addr peername() const;

    public:
        ssize_t send(const char * msg, size_t len) const;

        ssize_t send(const std::string & msg) const;

        ssize_t sendall(const std::string & msg) const;

        ssize_t recv(char * msg, size_t len) const;

        ssize_t recvall(std::string & msg) const;

    private:
        int createSock ();

        void connect(const Ip4Addr & ipaddr);

        std::shared_ptr<TcpConn> self() { return shared_from_this(); }

        void attach(int fd) { m_event.attach(fd); }

        inline int fd() const { return m_event.fd(); }

    private:
        Event           m_event;
        EventLoop &     m_loop;
        ConnState       m_state;
        TcpConnCallback m_readAction;
        TcpConnCallback m_writeAction;
        TcpConnCallback m_connectedAction;
    };

    class TcpServer: public std::enable_shared_from_this<TcpServer>, public BaseConn
    {
    public:
        static std::shared_ptr<TcpServer> startServer(EventLoop & loop, const std::string & ip, int port);

        TcpServer(EventLoop & loop);

        virtual ~TcpServer() = default;

        void bind(const std::string & ip, int port);
        void bind(const Ip4Addr & addr);

        void onClientAccepted(const TcpConnCallback & cb) { m_clientAcceptedAction = cb; }

        // close server fd and stop event loop
        void stopServer() { m_loop.remove(m_event); ::close(fd()); m_loop.stop(); }

    private:
        void attach(int fd) { m_event.attach(fd); }

        inline int fd() { return m_event.fd(); }

        std::shared_ptr<TcpConn> accept();

    private:
        Event                                       m_event;
        EventLoop &                                 m_loop;
        TcpConnCallback                             m_clientAcceptedAction;
        std::vector< std::shared_ptr<TcpConn> >     m_clients;
    };
}

#endif //TINYRPC_CONN_H
