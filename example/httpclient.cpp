#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "event.h"
#include "logger.h"
#include "conn.h"

using namespace tinynet;
using namespace std;

int main()
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
        })->onRead([&loop](shared_ptr<TcpConn> conn) {
            ssize_t ret = 0;
            stringstream ss;
            do
            {
                string html;
                ret = conn->recvall(html);
                ss << html;
            } while (ret == -1);

            info("%s", ss.str().c_str());

            conn->close();
            loop.stop();
        });

        loop.start();
    }
    catch (const util::TinyExp & e)
    {
        cout << e.toStr() << endl;
    }

    return 0;
}