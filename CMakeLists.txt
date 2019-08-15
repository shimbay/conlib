cmake_minimum_required(VERSION 3.5)
project(conlib)

file(GLOB_RECURSE SRCS "*.h")
include_directories("." "..")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14 -fPIC -ggdb -Wno-deprecated-declarations")

add_library(conlib.core OBJECT ${SRCS})
# compile hawkeye.core with -O3 flag
set_target_properties(conlib.core PROPERTIES
        COMPILE_FLAGS "-O3 -fPIC"
        )

add_executable(conlib.test test/main.cpp $<TARGET_OBJECTS:conlib.core>)