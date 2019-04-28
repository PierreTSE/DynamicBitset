cmake_minimum_required(VERSION 3.13)
project(DynamicBitset)

set(CMAKE_CXX_STANDARD 17)

include_directories("D:/Programmes/_CPP")

add_executable(unit_tests
        DynamicBitset.hpp
        test-DynamicBitset.cpp
        test-main.cpp)
