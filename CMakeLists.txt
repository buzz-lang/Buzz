cmake_minimum_required(VERSION 2.8.3)

project(buzz)

add_library(buzz SHARED
  buzzvm.h buzzvm.c
  buzzasm.h buzzasm.c)

add_executable(buzztest buzztest.c)
target_link_libraries(buzztest buzz)
