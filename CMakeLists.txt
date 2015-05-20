cmake_minimum_required(VERSION 2.8.3)

# Debug flags
set(CMAKE_C_FLAGS_DEBUG "-g -gdwarf-4")
set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-gdwarf-4")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "-gdwarf-4")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "-gdwarf-4")

# Project name
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
  buzztype.h buzztype.c
  buzzheap.h buzzheap.c
  buzzmsg.h buzzmsg.c
  buzzvstig.h buzzvstig.c
  buzzvm.h buzzvm.c
  buzzasm.h buzzasm.c
  buzzlex.h buzzlex.c
  buzzparser.h buzzparser.c)

# Create libbuzzargos
add_library(buzzargos SHARED
  buzz_controller.h buzz_controller.cpp)
target_link_libraries(buzzargos
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  buzz)

# Test programs
add_executable(testbuzzdarray testbuzzdarray.c)
target_link_libraries(testbuzzdarray buzz)

add_executable(testbuzzdict testbuzzdict.c)
target_link_libraries(testbuzzdict buzz)

add_executable(testbuzzmsg testbuzzmsg.c)
target_link_libraries(testbuzzmsg buzz)

add_executable(testbuzzlex testbuzzlex.c)
target_link_libraries(testbuzzlex buzz)

add_executable(testbuzzpar testbuzzpar.c)
target_link_libraries(testbuzzpar buzz)

# add_executable(buzzasm buzzasm.c)
# target_link_libraries(buzzasm buzz)

# add_executable(buzzc buzzc.c)
# target_link_libraries(buzzc buzz)

add_executable(testbuzz testbuzz.c)
target_link_libraries(testbuzz buzz)
