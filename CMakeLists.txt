cmake_minimum_required(VERSION 3.21)
project(libczh)
set(CMAKE_CXX_STANDARD 17)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(example
        examples/cpp/example.cpp)
