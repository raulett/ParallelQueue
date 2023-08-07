cmake_minimum_required(VERSION 3.26)
project(ParallelQueue)

set(CMAKE_CXX_STANDARD 11)

add_library(ParallelQueue STATIC ParallelQueue.cpp)
target_include_directories(ParallelQueue PUBLIC ${PROJECT_SOURCE_DIR})
