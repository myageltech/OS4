cmake_minimum_required(VERSION 3.25)

project(OS4_3)

set(CMAKE_CXX_STANDARD 11)

set (PROJECT_HEADERS
    my_stdlib.h
)

set (PROJECT_SOURCES
    test_3.cpp
    malloc_3.cpp 
)

add_executable(OS4_3 ${PROJECT_HEADERS} ${PROJECT_SOURCES} )
