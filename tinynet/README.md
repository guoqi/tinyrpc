# TinyNet - a simple network library

## Usage

### Simple event library
```(C++)
/*
 * Create a client socket (read/write) event
 *   Assume variable fd is a valid socket fd and it is set non-blocking flag.
 *   Before that, you should connect to a server with this fd but don't know if it is succeeded.
 * What a callback hell it is! We will fix it in the later with deferred.
 */
Event ev(fd);
ev.onWrite([](EventLoop & loop, Event & ev){
    // check if connect successfully
    
    ev.onRead([](EventLoop & loop, Event & ev){
        // read something from server and close fd
        ev.readable(false);
        loop.remove(ev);
        close(ev.fd());
    }).onWrite([](EventLoop & loop, Event & ev){
        // send something to server and disable write.
        ev.writeable(false);
    });
});

/**
 * Create an event loop and set a timer which acts after 30ms at first and runs every 100ms after that.
 */
EventLoop loop(100);
loop.runAfter(30, [](EventLoop & loop){
    cout << "hi, timer runs" << endl;
}, 100);

/*
 * Create a signal event - handle system signal
 */
Signal::signal(SIGINT, [&loop](int sig){
    cout << "receive a SIGINT signal" << endl;
    loop.stop()
});

/*
 * Handle multiple signals with the same handler 
 */
auto sigset = {SIGUSR1, SIGUSR2};
Signal::signal(sigset, [](int sig){
    switch (sig)
    {
        case SIGUSR1:
            cout << "receive a SIGUSR1 signal" << endl;
            break;
        case SIGUSR2:
            cout << "receive a SIGUSR2 signal" << endl;
            break;
        default:
            cout << "unexcepted signal" << endl;
    }
});

/*
 * Add event into the event loop and run
 */
// alter an event into event loop
loop.alter(ev);
loop.start(); // start the event loop
```

As shown above, we give typical examples about socket event, timer event and signal event.

When the event loop is run, these events run as the follow order:

1. First alter these event into event queeue and do nothing. (No event is done.)

1. Then register a signal handler to handle signal event. (No event is done either.)

1. Do a epoll loop whose tiemout is the smallest of all timer events.

1. Check if there is some sockets ready to read or write and call their corresponding callback function when epoll returns

1. If there is no socket ready and epoll is timeout, call timer events whose timeout equals epoll timeout. (There may be many timer events to be executed.)

1. Check if the event loop is stopped and exit, otherwise go to step 2


### Simple network library (only tcp support for now)
*See also example/echocli.cpp and example/echosvr.cpp*

#### Server-Side
```(C++)
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
```

#### Client-Side
```(C++)
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

        conn->readwrite(false, true); // switch to write

        if (ret == 0) // peer close
        {
            conn->close();
            loop.stop(); // capture the event loop
        }
    });

    conn->readwrite(false, true); // initial prepare for writing
});
```