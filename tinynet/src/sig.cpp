//
// Created by qiguo on 1/14/18.
//

#include <csignal>
#include "sig.h"

using namespace std;

namespace tinynet
{
    map<int, SignalCallback> Signal::m_sighandlers;

    void Signal::signal(int sig, const SignalCallback &cb)
    {
        sighandler_t ret = ::signal(sig, &Signal::dispatcher);
        fatalif(ret == SIG_ERR);

        m_sighandlers[sig] = cb;
    }

    void Signal::signal(const std::initializer_list<int> & sigset, const SignalCallback &cb)
    {
        for (auto const & sig : sigset)
        {
            signal(sig, cb);
        }
    }

    void Signal::ignore(int sig)
    {
        fatalif(::signal(sig, SIG_IGN) == SIG_ERR);

        if (m_sighandlers.find(sig) == m_sighandlers.end())
        {
            m_sighandlers.erase(sig);
        }
    }

    void Signal::remove(int sig)
    {
        fatalif(::signal(sig, SIG_DFL) == SIG_ERR);

        if (m_sighandlers.find(sig) == m_sighandlers.end())
        {
            m_sighandlers.erase(sig);
        }
    }

    void Signal::dispatcher(int sig)
    {
        if (m_sighandlers.find(sig) == m_sighandlers.end())
        {
            warn("no such signal handler for sig[%d]", sig);
            return;
        }

        auto const & handler = m_sighandlers.at(sig);
        handler(sig);
    }
}
