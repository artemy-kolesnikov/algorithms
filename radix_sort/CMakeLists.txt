cmake_minimum_required (VERSION 2.6)

set (radix_sort radix_sort)

#set (CMAKE_BUILD_TYPE "Release")
set (CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall --unroll-loops")

set (sources
    main.cpp)

include_directories(. ../index/util)

add_executable(${radix_sort} ${sources})
