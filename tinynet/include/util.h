/*
 * File: util.h
 * Author: qiguo
 * Date: 2017.12.10
 */
#ifndef __TINYNET_UTIL_H__
#define __TINYNET_UTIL_H__

#include <memory>
#include <exception>
#include <string>
#include <sstream>
#include <error.h>
#include <cstring>

/**
 * Macro for handling system call fails
 */
#define fatal()                 do { throw util::TinyExp(__LINE__, __FILE__); } while (false)
#define fatalif(r, cond)        do { if ((r) == (cond)) throw util::TinyExp(__LINE__, __FILE__); } while (false)
#define fatalnot(r, cond)       do { if ((r) != (cond)) throw util::TinyExp(__LINE__, __FILE__); } while (false)
#define returnif(r, cond)       do { if ((r) == (cond)) return -1; } while (false)
#define returnnot(r, cond)       do { if ((r) != (cond)) return -1; } while (false)

// some common utilities
namespace util
{
    // non-copy attribute
    struct noncopyable
    {
        noncopyable() = default;
        noncopyable(const noncopyable & obj) = delete;
        virtual ~noncopyable() = default;
    };

    struct TinyExp : public std::exception
    {
        TinyExp(const int error, const std::string & errmsg, int line, const char * file) noexcept
            : m_error(error), m_errmsg(errmsg), m_line(line), m_file(file) {}
        TinyExp(int line, const char * file) noexcept : TinyExp(errno, strerror(errno), line, file) {}

        virtual ~TinyExp() = default;

        virtual inline const char * what() const noexcept { return m_errmsg.c_str(); }
        virtual inline int error() const noexcept { return m_error; }

        std::string toStr() const noexcept
        {
            std::stringstream ss;
            ss << "TinyExp[" << m_file << "][" << m_line << "] - "
               << m_error << ": " << m_errmsg;
            return ss.str();
        }

    private:
        int             m_error;
        std::string     m_errmsg;
        int             m_line;
        std::string     m_file;
    };


    // Time related function
    namespace Time
    {
        // get current time (miloseconds format)
        int64_t nowMs();
        int64_t nowUs();
    }
}

#endif