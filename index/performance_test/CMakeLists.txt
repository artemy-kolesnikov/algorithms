cmake_minimum_required (VERSION 2.6)

set (performance_test performance_test)

set (sources
    main.cpp)

include_directories(../util)

add_executable(${performance_test} ${sources})
