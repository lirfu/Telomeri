cmake_minimum_required(VERSION 3.8)
project(Telomeri)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        include/greeter/Greeter.hpp
        include/old/Greeter.h
        src/greeter/Greeter.cpp
        src/old/Greeter.c
        src/Main.cpp
        test/CatchMain.cpp
        test/Tests.cpp)

add_executable(Telomeri ${SOURCE_FILES})