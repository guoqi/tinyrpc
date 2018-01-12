# TinyNet - a simple network library

## Usage

### Simple event library

```(C++)
/*
 * Create a socket (read/write) event
 *   Assume variable fd is a valid socket fd and it is set non-blocking flag.
 *   Parameter data represents user-side data which may be modified when every callback action runs.
 * What a callback hell it is! We will fix it in the later with deferred.
 */
SockEvent<T> scEvent(S_CONN, fd, data, [](auto & loop, auto & ev, T data) -> bool{
    // on connect event
    SockEvent<T> onRead(S_READ, fd, data, [](auto & loop, auto & ev, T data) -> bool{
        // on read event
        SocketEvent<T> onWrite(S_WRITE, fd, data, [](auto & loop, auto & ev, T data)-> bool{
            // on write event
            loop.alter(onClose);
        });
        loop.alter(onWrite);
    });
    loop.alter(onRead);
});

/*
 * Create a timer event which is accurate to millisecond
 */
TimerEvent tmEvent(10, [](auto & loop, auto & ev) -> bool {
    // do somthing after 10 ms
});

/*
 * Create a signal event - handle system signal
 */
SignalEvent sigEvent([](auto & loop, auto & ev, int sig) -> bool {
    // do something with variable sig
});

/*
 * Create a signal event on specific signal
 */
SignalEvent sigEvent(SIGHUP, [](auto & loop, auto & ev)->bool {
    // do something with specific signal SIGHUP
});

/*
 * Create an event loop and run
 */
auto & loop = new EventLoop();
// alter an event into event loop
loop.alter(scEvent);
loop.alter(tmEvent);
loop.alter(sigEvent);
loop.alter(sigEvent2);
loop.run(); // run the event loop
```

As shown above, we give typical examples about socket event, timer event and signal event.

When the event loop is run, these events run as the follow order:

1. First alter these event into event queeue and do nothing. (No event is done.)

1. Then register a signal handler to handle signal event. (No event is done either.)

1. Do a epoll loop whose tiemout is the smallest of all timer events.

1. Check if there is some sockets ready to read or write and call their corresponding callback function when epoll returns

1. If there is no socket ready and epoll is timeout, call timer events whose timeout equals epoll timeout. (There may be many timer events to be executed.)

1. Check if the event loop is stopped and exit, otherwise go to step 2