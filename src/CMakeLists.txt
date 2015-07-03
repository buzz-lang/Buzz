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
include(${CMAKE_SOURCE_DIR}/cmake/BuzzPackaging.cmake)

#
# Compile stuff
#
add_subdirectory(buzz)
add_subdirectory(testing)
add_subdirectory(utility)

#
# Install headers
#

#
# Install the README in doc/buzz
#
install(FILES ${CMAKE_SOURCE_DIR}/../README.md DESTINATION doc/buzz)
