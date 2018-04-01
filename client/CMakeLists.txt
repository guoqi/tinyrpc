cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(
        ${PROJECT_SOURCE_DIR}/tinynet/include
        ${PROJECT_SOURCE_DIR}/base/include
        ${PROJECT_SOURCE_DIR}/client/include
)

add_library(rpc_client OBJECT
        include/rpcconn.h
        src/rpcconn.cpp
        )
