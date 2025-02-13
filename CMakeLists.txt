cmake_minimum_required(VERSION 3.15)
project(wayhome VERSION 0.0.1 LANGUAGES C CXX)

SET(CMAKE_CXX_STANDARD 23)

add_subdirectory(src)
add_subdirectory(libs/argparser)
