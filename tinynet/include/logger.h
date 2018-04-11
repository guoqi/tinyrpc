//
// Created by qiguo on 1/1/18.
//

#ifndef __TINYRPC_LOGGER_H__
#define __TINYRPC_LOGGER_H__

#include <cstdio>
#include <map>
#include <string>
#include <pthread.h>
#include "util.h"

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
 * base log macro
 */
#define debug(fmt, ...)     log(stdout, util::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define info(fmt, ...)      log(stdout, util::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define warn(fmt, ...)      log(stderr, util::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define error(fmt, ...)     log(stderr, util::LogLevel::ERROR, fmt, ##__VA_ARGS__)

#define log(fd, level, fmt, ...) do { fprintf(fd, "[%s][%s][%u][%s][%d]" fmt "\n", \
                                        util::Time::datetime().c_str(), \
                                        util::LEVEL_STRING.at(level).c_str(), \
                                        pthread_self(), \
                                        __FILE__, \
                                        __LINE__, \
                                        ##__VA_ARGS__); } while (false)

namespace util
{
    enum class LogLevel {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4
    };

    const static std::map<LogLevel, std::string> LEVEL_STRING = {
            {LogLevel::DEBUG, "debug"},
            {LogLevel::INFO, "info"},
            {LogLevel::WARN, "warn"},
            {LogLevel::ERROR, "error"}
    };
}

#endif //TINYRPC_LOGGER_H
