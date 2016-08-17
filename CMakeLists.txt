cmake_minimum_required(VERSION 3.6)
project(unixSocketSendFD_log)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    recvfd.c
    )

add_executable(unixSocketSendFD_log ${SOURCE_FILES})