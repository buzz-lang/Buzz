cmake_minimum_required(VERSION 2.8.3)

project(buzz)

# Find the ARGoS package
find_package(PkgConfig)
pkg_check_modules(ARGOS REQUIRED argos3_simulator)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
include(ARGoSCheckQTOpenGL)
find_package(GSL REQUIRED)
find_package(Lua52 REQUIRED)
include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS} ${GSL_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
link_directories(${ARGOS_LIBRARY_DIRS})

# Create libbuzz
add_library(buzz SHARED
  buzzdarray.h buzzdarray.c
  buzzdict.h buzzdict.c
  buzzvm.h buzzvm.c
  buzzasm.h buzzasm.c
  buzzlex.h buzzlex.c
  buzzparser.h buzzparser.c)

# Create libbuzzargos
add_library(buzzargos SHARED
  buzz_controller.h buzz_controller.cpp)
target_link_libraries(buzzargos
  argos3core_simulator buzz)

# Test programs
add_executable(testbuzzdarray testbuzzdarray.c)
target_link_libraries(testbuzzdarray buzz)

add_executable(testbuzzdict testbuzzdict.c)
target_link_libraries(testbuzzdict buzz)

add_executable(buzztest buzztest.c)
target_link_libraries(buzztest buzz)

add_executable(testbuzzlex testbuzzlex.c)
target_link_libraries(testbuzzlex buzz)

add_executable(testbuzzpar testbuzzpar.c)
target_link_libraries(testbuzzpar buzz)
