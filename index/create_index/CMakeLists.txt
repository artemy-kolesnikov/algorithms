cmake_minimum_required (VERSION 2.6)

set (create_index create_index)

set (sources
    main.cpp)

include_directories(../util)

add_executable(${create_index} ${sources})
