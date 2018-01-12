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

    return 0;
}