//
// Created by qiguo on 1/1/18.
//

#ifndef __TINYRPC_LOGGER_H
#define __TINYRPC_LOGGER_H

#include <cstdio>
#include <map>
#include <string>

#define debug(fmt, ...)     log(stdout, util::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define info(fmt, ...)      log(stdout, util::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define warn(fmt, ...)      log(stderr, util::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define fault(fmt, ...)     log(stderr, util::LogLevel::ERROR, fmt, ##__VA_ARGS__)

#define log(fd, level, fmt, ...) do { fprintf(fd, "[%s][%s][%d]" fmt "\n", util::LEVEL_STRING.at(level).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); } while (false)

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
