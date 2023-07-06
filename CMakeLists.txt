cmake_minimum_required(VERSION 3.9)

project(OS4_2)

set(CMAKE_CXX_STANDARD 11)

set (PROJECT_HEADERS
    my_stdlib.h
)

set (PROJECT_SOURCES
    test_2.cpp
    malloc_2.cpp 
)

add_executable(OS4_2 ${PROJECT_SOURCES} ${PROJECT_HEADERS})
