cmake_minimum_required(VERSION 3.8.2)
project(libczh)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)
add_executable(libczh examples/cpp/example.cpp)

enable_testing()
add_subdirectory(tests)