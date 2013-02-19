cmake_minimum_required (VERSION 2.6)

set (test_index test_index)

set (sources
    main.cpp)

include_directories(../util)

add_executable(${test_index} ${sources})
