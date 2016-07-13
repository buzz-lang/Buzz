#
# Find pkg-config
#
find_package(PkgConfig REQUIRED)

#
# Look for the optional ARGoS package
#
pkg_check_modules(ARGOS argos3_simulator)
if(ARGOS_FOUND)
  include_directories(${ARGOS_INCLUDE_DIRS})
  link_directories(${ARGOS_LIBRARY_DIRS})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
  include(ARGoSCheckQTOpenGL)
  # Look for Lua52
  find_package(Lua52 REQUIRED)
  include_directories(${LUA_INCLUDE_DIR})
endif(ARGOS_FOUND)
