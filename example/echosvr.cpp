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

int main()
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
    return 0;
}
