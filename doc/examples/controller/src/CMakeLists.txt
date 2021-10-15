#
# Set minimum required version
#
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_BUILD_TYPE Debug)

#
# Options
#
option(ARGOS_SIMULATION "Build for ARGoS Simulation" ON)
#
# Find pkg-config
#
find_package(PkgConfig REQUIRED)

if(ARGOS_SIMULATION)
#
# Look for the optional ARGoS package
#
pkg_check_modules(ARGOS argos3_simulator)
if(ARGOS_FOUND)
  include_directories(${ARGOS_INCLUDE_DIRS})
  link_directories(${ARGOS_LIBRARY_DIRS})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
  include(ARGoSCheckQTOpenGL)
  # Look for Lua
  find_package(Lua REQUIRED)
  include_directories(${LUA_INCLUDE_DIR})
  link_directories(${ARGOS_PREFIX}/lib/argos3/)
endif(ARGOS_FOUND)

set(ARGOS_BUZZ_SOURCES
  utils/buzz_utils.h  
  argos/buzz_controller.h       
  argos/buzz_controller.cpp     
  argos/buzz_closures.cpp
  argos/radiation_source.h       
  argos/radiation_source.cpp)

add_library(argos3plugin_buzz_simulator_drone SHARED ${ARGOS_BUZZ_SOURCES})
target_link_libraries(argos3plugin_buzz_simulator_drone
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  argos3plugin_simulator_spiri
  argos3plugin_simulator_buzz
  buzz
  jsoncpp)

add_compile_options ( -g -rdynamic)
set_target_properties(argos3plugin_buzz_simulator_drone PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS argos3plugin_buzz_simulator_drone LIBRARY DESTINATION ${ARGOS_PREFIX}/lib/argos3)
install(DIRECTORY . DESTINATION include/buzz/argos FILES_MATCHING PATTERN "*.h")

endif(ARGOS_SIMULATION)
