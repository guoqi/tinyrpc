cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(
    ${PROJECT_SOURCE_DIR}/tinynet/include
    ${PROJECT_SOURCE_DIR}/base/include
    ${PROJECT_SOURCE_DIR}/server/include
)

set(SOURCE_FILES
        include/proxy.h
        src/proxy.cpp
        include/server.h
        src/server.cpp
        src/main.cpp
        )

add_library(rpc_server OBJECT ${SOURCE_FILES})
