cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -lpthread")

include_directories(
        ${PROJECT_SOURCE_DIR}/tinynet/include
        ${PROJECT_SOURCE_DIR}/base/include
)

add_library(base OBJECT
        include/protocol.h
        src/protocol.cpp
        include/pool.h
        include/thread.h
        src/thread.cpp
        include/config.h
        src/config.cpp
        include/ts_queue.h
        src/ts_queue.cpp
)


# for unit test!!!!
add_executable(test_base
            test/test_ts_queue.cpp
            include/ts_queue.h
            src/ts_queue.cpp
        )
