cmake_minimum_required(VERSION 3.5)

project(conlib)

find_package(gtest)

file(
  GLOB_RECURSE
  SRCS
  conlib/*.h
)
file(
  GLOB_RECURSE
  TESTS
  test/*.cpp
)

add_library(
  ${PROJECT_NAME}
  INTERFACE
  ${SRCS}
)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(
  ${PROJECT_NAME}
  INTERFACE -fPIC
            -ggdb
            -Wno-deprecated-declarations
)
target_link_libraries(${PROJECT_NAME} INTERFACE dl pthread)

add_executable(${PROJECT_NAME}_test ${TESTS})
target_link_libraries(
  ${PROJECT_NAME}_test
  gtest
  ${PROJECT_NAME}
)
