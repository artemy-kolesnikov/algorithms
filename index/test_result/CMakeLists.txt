cmake_minimum_required (VERSION 2.6)

set (test_result test_result)

set (sources
    main.cpp)

set (CMAKE_BUILD_TYPE "Release")
set (CMAKE_CXX_FLAGS "-std=c++11 -O3")

include_directories(../util)

add_executable(${test_result} ${sources})
