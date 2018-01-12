cmake_minimum_required(VERSION 3.2)
project(tinyrpc CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/tinynet/include
)

add_subdirectory(tinynet)

set(SOURCE_FILES main.cpp)
add_executable(tinyrpc ${SOURCE_FILES})

set_target_properties(tinynet PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(tinyrpc tinynet)
