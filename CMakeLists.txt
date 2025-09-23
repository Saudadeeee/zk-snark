cmake_minimum_required(VERSION 3.16)
project(zkmini VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create library
add_library(zkmini ${SOURCES})

# Apps
add_executable(zksetup apps/zksetup.cpp)
target_link_libraries(zksetup zkmini)

add_executable(zkprove apps/zkprove.cpp)
target_link_libraries(zkprove zkmini)

add_executable(zkverify apps/zkverify.cpp)
target_link_libraries(zkverify zkmini)

# Tests
enable_testing()

file(GLOB TEST_SOURCES "tests/test_*.cpp")
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} zkmini)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Install
install(TARGETS zkmini DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)