cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(
    ${PROJECT_SOURCE_DIR}/tinynet/include
    $(PROJECT_SOURCE_DIR)/proxy/include
    $(PROJECT_SOURCE_DIR)/config/include
)

add_library(proxy
        include/proxy.h
        src/proxy.cpp
)
