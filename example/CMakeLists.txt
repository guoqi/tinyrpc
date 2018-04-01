cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(TOP_PROJECT_DIR "${PROJECT_SOURCE_DIR}/../")

include_directories(
        ${TOP_PROJECT_DIR}tinynet/include
        ${TOP_PROJECT_DIR}tinyrpc/include
        ${TOP_PROJECT_DIR}base/include
        ${TOP_PROJECT_DIR}config/include
)

set(SOURCE_FILES
        rpcsvr/hello.cpp
        rpcsvr/hello.h)

link_directories("/home/qiguo/Toys/tinyrpc/build")

aux_source_directory(${TOP_PROJECT_DIR}/tinynet/src TINYNET)

add_executable(echosvr echosvr.cpp ${TINYNET})
add_executable(echocli echocli.cpp ${TINYNET})
add_executable(httpclient httpclient.cpp ${TINYNET})
add_executable(sigdemo sigdemo.cpp ${TINYNET})
add_executable(rpcsvr ${SOURCE_FILES})
target_link_libraries(rpcsvr tinyrpc pthread)
