#include <iostream>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "event.h"
#include "logger.h"
#include "conn.h"

using namespace tinynet;
using namespace std;

int create_sock()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(fd, -1);

    int flags = fcntl(fd, F_GETFL, 0);
    int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    fatalif(ret, -1);

    return fd;
}

void connect(int fd, const string & ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    int r = connect(fd, (sockaddr *)&addr, sizeof(addr));
    if (r == -1)
    {
        if (errno != EINPROGRESS)
        {
            perror("connect failed.");
            exit(1);
        }
    }
}

void test_1()
{
    try
    {
        int fd = create_sock();
        connect(fd, "111.13.101.208", 80);

        Event ev(fd);   // bind fd to sock event

        ev.onWrite([](EventLoop & loop, Event & ev){
            // check if connect is ready
            int err = 0;
            socklen_t errlen = sizeof(err);
            if (getsockopt(ev.fd(), SOL_SOCKET, SO_ERROR, &err, &errlen) == -1)
            {
                perror("getsockopt error");
                exit(1);
            }

            if (err)
            {
                printf("connect error: %d\n", err);
                exit(1);
            }

            // connect successfully
            ev.onRead([](EventLoop & loop, Event & ev){
                char buffer[8192] = {0};
                recv(ev.fd(), buffer, sizeof(buffer), 0);
                cout << buffer << endl;
                ev.readable(false);
                loop.remove(ev);
            }).onWrite([](EventLoop & loop, Event & ev){
                char buffer[512] = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
                send(ev.fd(), buffer, strlen(buffer), 0);
                cout << "send successfully~" << endl;
                ev.writeable(false);
            });

            loop.alter(ev);
        });

        EventLoop loop(1000);
        loop.alter(ev);
        loop.runAfter(10000, [](EventLoop & loop){
            cout << "timeout~" << endl;
        }, 100);
        loop.start();
    }
    catch (const util::TinyExp & e)
    {
        cout << e.toStr() << endl;
    }
}

void test_2()
{
    try
    {
        EventLoop loop(1000);

        auto client = TcpConn::createConnection(loop, "14.215.177.38", 80);

        client->onConnected([](shared_ptr<TcpConn> conn) {
            info("connected succ~");
            fflush(stdout);

            string buf = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
            conn->send(buf);
            conn->closeWrite();
        })->onRead([](shared_ptr<TcpConn> conn) {
            ssize_t ret = 0;
            do
            {
                string html;
                ret = conn->recvall(html);
                info("%s", html.c_str());
            } while (ret == -1);

            conn->close();
        });

        loop.start();
    }
    catch (const util::TinyExp & e)
    {
        cout << e.toStr() << endl;
    }
}

void client()
{
    try
    {
        EventLoop loop(100);

        auto client = TcpConn::createConnection(loop, "127.0.0.1", 9999);

        client->onConnected([&loop](shared_ptr<TcpConn> conn){
            info("connected");
            conn->onWrite([](shared_ptr<TcpConn> conn){
                string data;
                if (getline(cin, data))
                {
                    conn->send(data);
                }
                else
                {
                    conn->closeWrite();
                }
                conn->readwrite(true, false); // switch to read

            })->onRead([&loop](shared_ptr<TcpConn> conn){
                string data;
                ssize_t ret = conn->recvall(data);
                info("%s", data.c_str());
                fflush(stdout);

                conn->readwrite(false, true); // switch to write

                if (ret == 0) // peer close
                {
                    conn->close();
                    loop.stop(); // capture the event loop
                }
            });

            conn->readwrite(false, true); // initial prepare for writing
        });

        loop.start();
    }
    catch (const util::TinyExp & e)
    {
        cout << e.toStr() << endl;
    }
}

void server()
{
    try
    {
        EventLoop loop(100);

        auto server = TcpServer::startServer(loop, "127.0.0.1", 9999);

        server->onClientAccepted([&server](shared_ptr<TcpConn> conn){
            conn->onRead([&server](shared_ptr<TcpConn> conn){
                string data;
                ssize_t ret = conn->recvall(data);
                info("%s", data.c_str());
                fflush(stdout);

                conn->send(data);

                if (ret == 0)
                {
                    conn->close();
                    server->stopServer();
                }
            });
        });

        loop.start();
    }
    catch (const util::TinyExp & e)
    {
        cout << e.toStr() << endl;
    }
}

int main(int argc, char * argv[])
{
    // test_1();

    // test_2();

    if (argc != 2)
    {
        cout << "Usage: ./tinyrpc <client/server>" << endl;
        exit(0);
    }

    if (string(argv[1]) == "client")
    {
        client();
    }
    else if (string(argv[1]) == "server")
    {
        server();
    }
    else
    {
        cout << "Usage: ./tinyrpc <client/server>" << endl;
        exit(0);
    }
    return 0;
}