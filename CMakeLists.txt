cmake_minimum_required(VERSION 3.5)
project(conlib)

# Set pkg-config dependencies
set(deps GTEST)
set(dep_pkg_names gtest)

find_package(PkgConfig REQUIRED)
list(LENGTH deps deps_size)
list(LENGTH dep_pkg_names dep_names_size)
if (NOT deps_size EQUAL dep_names_size)
    message(FATAL_ERROR "Dependencies size (${deps_size}) must be equal to dependency pkg-config names size (${dep_names_size})")
endif ()

# Check pkg_config deps
include_directories(".")
set(COMMON_LINK_LIBRARIES dl pthread)
math(EXPR deps_size "${deps_size} - 1")
foreach (i RANGE ${deps_size})
    list(GET deps ${i} dep)
    list(GET dep_pkg_names ${i} name)

    pkg_check_modules(${dep} REQUIRED ${name})
    include_directories(${${dep}_INCLUDE_DIRS})
    link_directories(${${dep}_LIBRARY_DIRS})
    list(APPEND COMMON_LINK_LIBRARIES ${${dep}_LIBRARIES})
    message("${dep} Include Dir: ${${dep}_INCLUDE_DIRS}")
    message("${dep} Lib Dir: ${${dep}_LIBRARY_DIRS}")
    message("${dep} Libs: ${${dep}_LIBRARIES}")
endforeach (i)

file(GLOB_RECURSE SRCS conlib/*.h)
file(GLOB_RECURSE TESTS test/*.cpp)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14 -fPIC -ggdb -Wno-deprecated-declarations")

add_library(conlib.core OBJECT ${SRCS})
set_target_properties(conlib.core PROPERTIES
        COMPILE_FLAGS "-O3 -fPIC"
        )

add_executable(conlib.test ${TESTS} $<TARGET_OBJECTS:conlib.core>)
target_link_libraries(conlib.test ${COMMON_LINK_LIBRARIES})
