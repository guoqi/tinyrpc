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
#include "logger.h"

/**
 * Macro for handling fatal errors. Use exception and can not be ignored
 * fatal family is used on system call error (like bad file descriptor, invalid params and so on)
 * panic family is used on self-defined error
 */
#define fatal()                     do { error("fatal [%d:%s]", errno, strerror(errno)); throw util::SysExp(__LINE__, __FILE__); } while (false)
#define fatalif(cond)               do { if ((cond)) { error("fatal [%d:%s] with cond", errno, strerror(errno)); throw util::SysExp(__LINE__, __FILE__); } } while (false)
#define panic(err, msg)             do { error("panic [%d:%s]", err, msg); throw util::TinyExp(err, msg, __LINE__, __FILE__); } while (false)
#define panicif(cond, err, msg)     do { if ((cond)) { error("panic [%d:%s]", err, msg); throw util::TinyExp(err, msg, __LINE__, __FILE__); } } while (false)
#define returnif(cond, r)           do { if ((cond)) return r; } while (false)

/**
 * Macro for handling non-fatal fails
 * Just record the log if the cond satisfies
 */
#define debugif(cond, fmt, ...)           do { if((cond)) debug(fmt, ##__VA_ARGS__); } while(false)
#define infoif(cond, fmt, ...)            do { if((cond)) info(fmt, ##__VA_ARGS__); } while(false)
#define warnif(cond, fmt, ...)            do { if((cond)) warn(fmt, ##__VA_ARGS__); } while(false)
#define errorif(cond, fmt, ...)           do { if((cond)) error(fmt, ##__VA_ARGS__); } while(false)

/**
 * Wrap normal system call and do something when returns withnot error
 * Note: stmt must be a statement or an expression otherwise you will get a compile error
 */
#define WITH(cond, stmt)                  do { if((cond)) {fmt} } while(false)

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

    // basic exception
    struct TinyExp : public std::exception
    {
        TinyExp(const int error, const std::string & errmsg, int line, const char * file) noexcept
            : m_error(error), m_errmsg(errmsg), m_line(line), m_file(file) {}

        virtual ~TinyExp() = default;

        virtual inline const char * what() const noexcept { return m_errmsg.c_str(); }
        virtual inline int code() const noexcept { return m_error; }

        std::string toStr() const noexcept
        {
            std::stringstream ss;
            ss << "TinyExp[" << m_file << "][" << m_line << "] - "
               << code() << ": " << what();
            return ss.str();
        }

    private:
        int             m_error;
        std::string     m_errmsg;
        int             m_line;
        std::string     m_file;
    };

    // linux system call exception
    // error code is global variable errno and error message is strerror(errno)
    struct SysExp : public TinyExp
    {
        SysExp (int line, const char * file) noexcept : TinyExp(errno, strerror(errno), line, file) {}
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