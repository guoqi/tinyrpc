cmake_minimum_required(VERSION 3.2)
project(tinyrpc CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/tinynet/include
    ${PROJECT_SOURCE_DIR}/config/include
    ${PROJECT_SOURCE_DIR}/proxy/include
)

add_subdirectory(tinynet)
add_subdirectory(proxy)
add_subdirectory(config)

set(SOURCE_FILES main.cpp)
add_executable(tinyrpc ${SOURCE_FILES})

set_target_properties(tinynet PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(proxy PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(config PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(tinyrpc tinynet)
target_link_libraries(tinyrpc proxy)
target_link_libraries(tinyrpc config)
