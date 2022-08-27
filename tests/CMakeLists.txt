cmake_minimum_required(VERSION 3.8.2)
project(libczh)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/googletest build)
set(GTEST_LIB gtest gtest_main)

add_executable(alltests  alltests.cpp)
target_link_libraries(alltests gtest)
add_test(NAME alltests COMMAND alltests)
