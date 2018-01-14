//
// Created by qiguo on 1/13/18.
//

#ifndef __TINYRPC_SIG_H__
#define __TINYRPC_SIG_H__

#include <string>
#include <functional>
#include <signal.h>
#include <initializer_list>
#include <map>
#include "util.h"

namespace tinynet
{
    using SignalCallback = std::function<void(int)>;

    class Signal
    {
    public:
        Signal() = delete;
        ~Signal() = default;

        static void signal(int sig, const SignalCallback & cb);

        static void signal(const std::initializer_list<int> & sigset, const SignalCallback & cb);

        static void ignore(int sig);

        static void remove(int sig);

    private:
        static void dispatcher(int sig);

    private:
        static std::map<int, SignalCallback>  m_sighandlers;
    };
}

#endif //TINYRPC_SIGNAL_H
