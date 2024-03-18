#
# Set minimum required version
#
cmake_minimum_required(VERSION 3.9)

#
# Project name
#
project(buzz)


#
# Set the compilation type
#
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ..." FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

#
# RPATH configuration
# Taken from https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

#
# Set the path additional cmake files must be searched for
#
set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

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
include(${CMAKE_SOURCE_DIR}/cmake/BuzzPackaging.cmake)

#
# Compile stuff
#
add_subdirectory(buzz)
add_subdirectory(testing)
add_subdirectory(utility)
add_subdirectory(include)

#
# Install headers
#

#
# Install the README in doc/buzz
#
install(FILES ${CMAKE_SOURCE_DIR}/../README.md DESTINATION doc/buzz)
