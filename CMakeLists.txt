cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

project(Goertzilla)
add_library(Goertzilla STATIC Goertzilla.cpp)
target_include_directories(Goertzilla PUBLIC ${CMAKE_CURRENT_LIST_DIR})
