//
// Created by qiguo on 1/4/18.
//

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>
#include "conn.h"
#include "logger.h"
#include "errlist.h"
#include "net.h"

using namespace std;

namespace tinynet
{
    TcpConn::TcpConn(EventLoop &loop)
        : m_loop(loop), m_state(ConnState::INIT)
    {
    }

    TcpConn::TcpConn(EventLoop &loop, const std::string &ip, int port)
        : TcpConn (loop, Ip4Addr(ip, port))
    {
    }

    TcpConn::TcpConn(EventLoop &loop, const Ip4Addr &addr)
        : TcpConn (loop)
    {
        m_addr = make_shared<Ip4Addr>(addr);

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        fatalif(fd == -1);

        net::setNonBlocking(fd);
        attach(fd);
        connect();
    }

    TcpConn::TcpConn(EventLoop & loop, const string & sockpath)
        : TcpConn (loop, UdsAddr(sockpath))
    {
    }

    TcpConn::TcpConn(EventLoop &loop, const UdsAddr &addr)
        : TcpConn(loop)
    {
        m_addr = make_shared<UdsAddr>(addr);

        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        fatalif(fd == -1);

        net::setNonBlocking(fd);
        attach(fd);
        connect();
    }

    ssize_t TcpConn::recvall(std::string &msg) const
    {
        ssize_t len = 0;
        do
        {
            char buffer[8192] = {0};
            len = recv(buffer, sizeof(buffer) - 1);
            if (len < 0)
            {
                // When we need recv all data, it always ends with a EAGAIN error which means there is no data left.
                // We just ommit the error in this case.
                break;
            }
            msg.append(string(buffer, len));
        } while (len > 0);

        return msg.length();
    }

    ssize_t TcpConn::recv(char *msg, size_t len) const
    {
        ssize_t ret = ::recv(fd(), msg, len, 0);
        if (ret < 0 && ! (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            fatalif(ret == -1); // throw an exception when occurs other error except that socket buffer has been full.
        }
        return ret;
    }

    ssize_t TcpConn::sendall(const std::string &msg) const
    {
        size_t total = 0;
        do
        {
            ssize_t len = send(msg.data() + total, msg.length() - total);
            if (len < 0)
            {
                break;  // There may be no left space (socket buffer) to send data, it is necessary to check whether all messages have been sent.
            }
            total += len;
        } while (total >= msg.length());

        return total;
    }

    ssize_t TcpConn::send(const std::string &msg) const
    {
        return send(msg.c_str(), msg.length());
    }

    ssize_t TcpConn::send(const char *msg, size_t len) const
    {
        ssize_t ret = ::send(fd(), msg, len, 0);
        if (ret < 0 && ! (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            fatalif(ret == -1);
        }
        return ret;
    }

    shared_ptr<TcpConn> TcpConn::onRead(const TcpConnCallback &cb)
    {
        m_readAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onRead([s](EventLoop & loop, Event & ev){
            panicif(s.expired(), ERR_INVALID_POINTER, "object has been released");

            auto self = s.lock();

            try
            {
                self->m_state = ConnState::READ;
                self->m_readAction(self);
            }
            catch (util::TinyExp & e)
            {
                self->m_state = ConnState::FAIL;
                throw;
            }
        });


        m_loop.alter(m_event);

        return self();
    }

    shared_ptr<TcpConn> TcpConn::onWrite(const TcpConnCallback &cb)
    {
        m_writeAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onWrite([s](EventLoop & loop, Event & ev){
            panicif(s.expired(), ERR_INVALID_POINTER, "object has been released");

            auto self = s.lock();

            try
            {
                self->m_state = ConnState::WRITE;
                self->m_writeAction(self);
            }
            catch (util::TinyExp & e)
            {
                self->m_state = ConnState::FAIL;
                throw;
            }
        });

        m_loop.alter(m_event);

        return self();
    }

    shared_ptr<TcpConn> TcpConn::onConnected(const TcpConnCallback &cb)
    {
        m_connectedAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onWrite([s](EventLoop & loop, Event & ev){
            panicif(s.expired(), ERR_INVALID_POINTER, "object has been released");

            // check if connect is ready

            int err = 0;
            socklen_t errlen = sizeof(err);

            auto self = s.lock();

            try
            {
                fatalif(getsockopt(self->fd(), SOL_SOCKET, SO_ERROR, &err, &errlen) == -1);

                errno = err;
                fatalif(err != 0);

                self->m_state = ConnState::CONN;

                loop.remove(ev);

                self->m_connectedAction(self);
            }
            catch (util::TinyExp & e)
            {
                self->m_state = ConnState::FAIL;
                throw;
            }
        });

        m_loop.alter(m_event);

        return self();
    }

    void TcpConn::connect()
    {
        int r = ::connect(fd(), m_addr->addr(), (socklen_t)m_addr->len());
        if (r == -1)
        {
            if (errno != EINPROGRESS)
            {
                fatal();
            }
       }
    }

    Ip4Addr TcpConn::peername() const
    {
        sockaddr_in sa;
        socklen_t len = sizeof(sa);
        if (getpeername(fd(), (sockaddr *)&sa, &len))
        {
            error("get peername error [%d][%s]", errno, strerror(errno));
            return Ip4Addr();
        }

        return Ip4Addr(sa);
    }

    std::shared_ptr<TcpConn> TcpConn::createConnection(EventLoop &loop, const std::string ip, int port)
    {
        return createConnection(loop, Ip4Addr(ip, port));
    }

    std::shared_ptr<TcpConn> TcpConn::createConnection(EventLoop &loop, const Ip4Addr &addr)
    {
        auto conn = make_shared<TcpConn>(loop, addr);
        return conn;
    }

    std::shared_ptr<TcpConn> TcpConn::createConnection(EventLoop &loop, const std::string sockpath)
    {
        return createConnection(loop, UdsAddr(sockpath));
    }

    std::shared_ptr<TcpConn> TcpConn::createConnection(EventLoop &loop, const UdsAddr &addr)
    {
        auto conn = make_shared<TcpConn>(loop, addr);
        return conn;
    }

    std::shared_ptr<TcpConn> TcpConn::createAttacher(EventLoop &loop, int fd)
    {
        auto conn = make_shared<TcpConn>(loop);
        conn->attach(fd);
        return conn;
    }

    TcpServer::TcpServer(EventLoop &loop)
        : m_loop(loop)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        fatalif(fd == -1);
        net::setNonBlocking(fd);

        attach(fd);
    }

    void TcpServer::bind(const std::string &ip, int port)
    {
        bind(Ip4Addr(ip, port));
    }

    void TcpServer::bind(const Ip4Addr &addr)
    {
        bind(addr.addr(), addr.len());
    }

    void TcpServer::bind(const std::string &sockpath)
    {
        bind(UdsAddr(sockpath));
    }

    void TcpServer::bind(const UdsAddr &addr)
    {
        unlink(addr.path().c_str());    // remove if exists
        bind(addr.addr(), addr.len());
    }

    void TcpServer::bind(sockaddr *addr, size_t len)
    {
        fatalif(::bind(fd(), addr, len) == -1);

        fatalif(listen(fd(), 100) == -1);

        weak_ptr<TcpServer> s = shared_from_this();

        m_event.onRead([s](EventLoop & loop, Event & ev){
            panicif(s.expired(), ERR_INVALID_POINTER, "object has been released");

            auto self = s.lock();
            auto client = self->accept();
            auto peer = client->peername();

            if (peer.valid())
            {
                info("accept client from [%s:%d]", peer.ip().c_str(), peer.port());
            }

            self->m_clients.push_back(client);
            self->m_clientAcceptedAction(client);
        });

        m_loop.alter(m_event);
    }

    std::shared_ptr<TcpConn> TcpServer::accept()
    {
        struct sockaddr addr;
        memset(&addr, 0, sizeof(sockaddr));
        socklen_t addrlen = 0;

        int cfd = ::accept(fd(), &addr, &addrlen);
        fatalif(cfd == -1);

        net::setNonBlocking(cfd);

        return TcpConn::createAttacher(m_loop, cfd);
    }

    std::shared_ptr<TcpServer> TcpServer::startServer(EventLoop &loop, const std::string &ip, int port)
    {
        auto server = make_shared<TcpServer> (loop);
        server->bind(ip, port);
        return server;
    }

    std::shared_ptr<TcpServer> TcpServer::startServer(EventLoop &loop, const std::string &sockpath)
    {
        auto server = make_shared<TcpServer> (loop);
        server->bind(sockpath);
        return server;
    }

}
