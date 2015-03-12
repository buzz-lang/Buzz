cmake_minimum_required(VERSION 2.8.3)

project(buzz)

add_library(buzzvm SHARED buzzvm.h buzzvm.c)

add_executable(buzzasm buzzasm.c)
target_link_libraries(buzzasm buzzvm)
