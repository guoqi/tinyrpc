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
#include "net.h"

using namespace std;

namespace tinynet
{
    TcpConn::TcpConn(EventLoop &loop)
        : m_loop(loop), m_state(ConnState::INIT)
    {
    }

    TcpConn::TcpConn(EventLoop &loop, const std::string &ip, int port)
        : TcpConn (loop)
    {
        int fd = createSock();
        attach(fd);
        connect(ip, port);
    }


    ssize_t TcpConn::recvall(std::string &msg)
    {
        ssize_t len = 0;
        do
        {
            char buffer[8192] = {0};
            len = recv(buffer, sizeof(buffer) - 1);
            if (len < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return -1;
                }
                fatalif(len, -1);
            }
            msg.append(string(buffer, len));
        } while (len > 0);

        return msg.length();
    }

    ssize_t TcpConn::recv(char *msg, size_t len)
    {
        return ::recv(fd(), msg, len, 0);
    }

    ssize_t TcpConn::sendall(const std::string &msg)
    {
        size_t total = 0;
        do
        {
            ssize_t len = send(msg.data() + total, msg.length() - total);
            if (len < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    return -1;  // socket buffer is full
                }
                fatalif(len, -1);
            }
            total += len;
        } while (total >= msg.length());

        return total;
    }

    ssize_t TcpConn::send(const std::string &msg)
    {
        return send(msg.c_str(), msg.length());
    }

    ssize_t TcpConn::send(const char *msg, size_t len)
    {
        return ::send(fd(), msg, len, 0);
    }

    shared_ptr<TcpConn> TcpConn::onRead(const TcpConnCallback &cb)
    {
        m_readAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onRead([s](EventLoop & loop, Event & ev){
            fatalif(s.expired(), true);

            auto self = s.lock();

            self->m_readAction(self);
        });


        m_loop.alter(m_event);

        return self();
    }

    shared_ptr<TcpConn> TcpConn::onWrite(const TcpConnCallback &cb)
    {
        m_writeAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onWrite([s](EventLoop & loop, Event & ev){
            fatalif(s.expired(), true);

            auto self = s.lock();

            self->m_writeAction(self);
        });

        m_loop.alter(m_event);

        return self();
    }

    shared_ptr<TcpConn> TcpConn::onConnected(const TcpConnCallback &cb)
    {
        m_connectedAction = cb;

        weak_ptr<TcpConn> s = self();

        m_event.onWrite([s](EventLoop & loop, Event & ev){
            fatalif(s.expired(), true);

            // check if connect is ready
            int ret = 0;
            int err = 0;
            socklen_t errlen = sizeof(err);

            auto self = s.lock();

            ret = getsockopt(self->fd(), SOL_SOCKET, SO_ERROR, &err, &errlen);
            fatalif(ret, -1);

            fatalnot(err, 0);

            self->m_state = ConnState::CONN;

            loop.remove(ev);

            self->m_connectedAction(self);
        });

        m_loop.alter(m_event);

        return self();
    }

    int TcpConn::createSock()
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        fatalif(fd, -1);

        net::setNonBlocking(fd);

        return fd;
    }

    void TcpConn::connect(const std::string &ip, int port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());

        int r = ::connect(fd(), (sockaddr *) &addr, sizeof(addr));
        if (r == -1)
        {
            if (errno != EINPROGRESS)
            {
                fatal();
            }
       }
    }

    std::shared_ptr<TcpConn> TcpConn::createConnection(EventLoop &loop, const std::string ip, int port)
    {
        auto conn = make_shared<TcpConn>(loop, ip, port);
        return conn;
    }

    std::shared_ptr<TcpConn> TcpConn::createAttacher(EventLoop &loop, int fd)
    {
        auto conn = make_shared<TcpConn>(loop);
        conn->attach(fd);
        debug("attacher");
        fflush(stdout);
        return conn;
    }

    TcpServer::TcpServer(EventLoop &loop)
        : m_loop(loop)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        fatalif(fd, -1);
        net::setNonBlocking(fd);

        attach(fd);
    }

    void TcpServer::bind(const std::string &ip, int port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());

        int r = ::bind(fd(), (sockaddr *) & addr, sizeof(addr));
        fatalif(r, -1);

        r = listen(fd(), 100);
        fatalif(r, -1);

        weak_ptr<TcpServer> s = shared_from_this();

        m_event.onRead([s](EventLoop & loop, Event & ev){
            fatalif(s.expired(), true);

            auto self = s.lock();
            auto client = self->accept();
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
        fatalif(cfd, -1);

        net::setNonBlocking(cfd);

        return TcpConn::createAttacher(m_loop, cfd);
    }

    std::shared_ptr<TcpServer> TcpServer::startServer(EventLoop &loop, const std::string &ip, int port)
    {
        auto server = make_shared<TcpServer> (loop);
        server->bind(ip, port);
        return server;
    }


}