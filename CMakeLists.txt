cmake_minimum_required(VERSION 3.2)
project(tinyrpc CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${PROJECT_SOURCE_DIR}/tinynet/include
    ${PROJECT_SOURCE_DIR}/config/include
    ${PROJECT_SOURCE_DIR}/base/include
)

add_subdirectory(tinynet)
add_subdirectory(config)
add_subdirectory(base)
add_subdirectory(tinyrpc)
