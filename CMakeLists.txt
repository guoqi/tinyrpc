cmake_minimum_required(VERSION 3.2)
project(tinyrpc CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(tinynet)
add_subdirectory(base)
add_subdirectory(server)
add_subdirectory(client)

add_library(rpcsvr STATIC $<TARGET_OBJECTS:tinynet>
                          $<TARGET_OBJECTS:base>
                          $<TARGET_OBJECTS:rpc_server>)

add_library(rpccli STATIC $<TARGET_OBJECTS:tinynet>
                          $<TARGET_OBJECTS:base>
                          $<TARGET_OBJECTS:rpc_client>)


enable_testing()
add_test(test_base ./base/test_base)
