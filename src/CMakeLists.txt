#
# Set minimum required version
#
cmake_minimum_required(VERSION 2.8.3)

#
# Set the path additional cmake files must be searched for
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

#
# Set CMake policies to select wanted behaviors
#
# Use new policies introduced up to this version
cmake_policy(VERSION 2.8.12)
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif(POLICY CMP0042)

#
# Include path points to the base source dir
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

#
# Set build options
#
include(${CMAKE_SOURCE_DIR}/cmake/BuzzBuildOptions.cmake)

#
# Set compiler flags
#
include(${CMAKE_SOURCE_DIR}/cmake/BuzzBuildFlags.cmake)

#
# Check for libraries
#
include(${CMAKE_SOURCE_DIR}/cmake/BuzzBuildChecks.cmake)

#
# Set up CPack for later use
#
#include(${CMAKE_SOURCE_DIR}/cmake/BuzzPackaging.cmake)

#
# Create config file with settings that must be accessed from C
#
# configure_file(
#   ${CMAKE_SOURCE_DIR}/buzz/config.h.in
#   ${CMAKE_BINARY_DIR}/buzz/config.h
#   @ONLY)
# install(FILES ${CMAKE_BINARY_DIR}/buzz/config.h DESTINATION include/buzz)

#
# Compile stuff
#
add_subdirectory(buzz)
if(ARGOS_FOUND)
  add_subdirectory(argos)
endif(ARGOS_FOUND)
add_subdirectory(testing)
