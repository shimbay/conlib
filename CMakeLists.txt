cmake_minimum_required(VERSION 3.5)
project(conlib)

file(GLOB_RECURSE SRCS "*.h" "test/main.cpp")
include_directories(".")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14 -fPIC -ggdb -Wno-deprecated-declarations")

add_library(conlib SHARED ${SRCS})