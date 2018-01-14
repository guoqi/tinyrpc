cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(
    ${PROJECT_SOURCE_DIR}/tinynet/include
)

add_library(tinynet
        include/channel.h
        include/conn.h
        include/event.h
        include/logger.h
        include/net.h
        include/poller.h
        include/util.h
        src/conn.cpp
        src/event.cpp
        src/net.cpp
        src/util.cpp
        include/errlist.h
        include/sig.h
        src/sig.cpp)
