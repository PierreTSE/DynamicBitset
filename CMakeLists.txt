cmake_minimum_required(VERSION 3.12)
project(DynamicBitset)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CATCH_DIR})

file(GLOB_RECURSE TEST_FILES test/*)

add_executable(unit_tests DynamicBitset.hpp test-DynamicBitset.cpp test-main.cpp ${TEST_FILES})

