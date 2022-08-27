cmake_minimum_required(VERSION 3.8.2)
project(libczh)
set(CMAKE_CXX_STANDARD 17)
add_executable(example examples/cpp/example.cpp)

enable_testing()
add_subdirectory(tests)
