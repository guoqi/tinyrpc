cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

add_library(base OBJECT
        include/rpcconn.h
        src/rpcconn.cpp
        include/protocol.h
        src/protocol.cpp
        include/pool.h
        include/thread.h
        src/thread.cpp
        include/config.h
        src/config.cpp
)
