cmake_minimum_required(VERSION 3.8)
project(Telomeri)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/Main.cpp
        src/OverlapGraph.cpp)

include_directories(include/ test/)

add_executable(Telomeri ${SOURCE_FILES})
