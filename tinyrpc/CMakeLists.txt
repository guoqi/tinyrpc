cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(
    ${PROJECT_SOURCE_DIR}/tinynet/include
    ${PROJECT_SOURCE_DIR}/config/include
    ${PROJECT_SOURCE_DIR}/tinyrpc/include
)

set(SOURCE_FILES
        include/proxy.h
        src/proxy.cpp
        include/server.h
        src/server.cpp
        src/main.cpp
        )

#add_executable(proxy ${SOURCE_FILES})

add_library(rpccore OBJECT ${SOURCE_FILES})

#set_target_properties(tinynet PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(config PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(base PROPERTIES LINKER_LANGUAGE CXX)

#target_link_libraries(proxy tinynet)
#target_link_libraries(proxy config)
#target_link_libraries(proxy base)
