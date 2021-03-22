cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_definitions(-std=c++11)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")


project(ring_buffer_library DESCRIPTION "C++ implementation of a ring buffer")

add_library(${PROJECT_NAME} SHARED ring_buffer.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ring_buffer.h)
