cmake_minimum_required(VERSION 3.8.2)
project(libczh)
add_executable(${PROJECT_NAME} tests.cpp)
target_link_libraries(${PROJECT_NAME} gtest)
