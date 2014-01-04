cmake_minimum_required (VERSION 2.6)

set (sort sort)

set (CMAKE_CXX_FLAGS -std=c++11)

set (sources
    main.cpp)

include_directories(../util)

add_executable(${sort} ${sources})
