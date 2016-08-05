# .rst:
# FindBuzz
# --------
#
# Find Module for Buzz
#
# This modules finds if Buzz is installed and determines where the
# tools, include files and libraries are.
#
# This module set the following result variables:
#
# ::
#
#   BUZZ_COMPILER        = The full path of bzzc
#   BUZZ_PARSER          = The full path of bzzparse
#   BUZZ_ASSEMBLER       = The full path of bzzasm
#   BUZZ_LIBRARY         = The full path of the Buzz library
#   BUZZ_C_INCLUDE_DIR   = The full path to the .h include files
#   BUZZ_BZZ_INCLUDE_DIR = The full path to the .bzz include files
#
# Examples Usages:
#
# ::
#
#   find_package(Buzz)
#   if(BUZZ_FOUND)
#     include_directories(${BUZZ_C_INCLUDE_DIR})
#     ...
#     target_link_libraries(... ${BUZZ_LIBRARY})
#   endif(BUZZ_FOUND)
#
#   find_package(Buzz REQUIRED)
#   include_directories(${BUZZ_C_INCLUDE_DIR})
#   ...
#   target_link_libraries(... ${BUZZ_LIBRARY})

#=============================================================================
# Copyright 2016 Carlo Pinciroli <carlo@pinciroli.net>
#=============================================================================

#
# Stardard Buzz tool paths
#
set(_BUZZ_TOOL_PATHS
  /usr/bin
  /usr/local/bin
  /opt/bin
  /opt/local/bin)

#
# Standard Buzz library paths
#
set(_BUZZ_LIBRARY_PATHS
  /usr/lib
  /usr/local/lib
  /opt/lib
  /opt/local/lib)

#
# Standard Buzz C include paths
#
set(_BUZZ_C_INCLUDE_PATHS
  /usr/include
  /usr/local/include
  /opt/include
  /opt/local/include)

#
# Standard Buzz BZZ include paths
#
set(_BUZZ_BZZ_INCLUDE_PATHS
  /usr/share/buzz/include
  /usr/local/share/buzz/include
  /opt/share/buzz/include
  /opt/local/share/buzz/include)

#
# Look for bzzc
#
find_program(BUZZ_COMPILER
  NAMES bzzc
  PATHS ${_BUZZ_TOOL_PATHS}
  DOC "Location of the bzzc compiler")

#
# Look for bzzparse
#
find_program(BUZZ_PARSER
  NAMES bzzparse
  PATHS ${_BUZZ_TOOL_PATHS}
  DOC "Location of the bzzparse compiler")

#
# Look for bzzasm
#
find_program(BUZZ_ASSEMBLER
  NAMES bzzasm
  PATHS ${_BUZZ_TOOL_PATHS}
  DOC "Location of the bzzasm compiler")

#
# Look for Buzz library
#
find_library(BUZZ_LIBRARY
  NAMES buzz
  PATHS ${_BUZZ_LIBRARY_PATHS}
  DOC "Location of the Buzz library")

#
# Look for Buzz library
#
find_library(BUZZ_LIBRARY_DEBUG
  NAMES buzzdbg
  PATHS ${_BUZZ_LIBRARY_PATHS}
  DOC "Location of the Buzz debug library")

#
# Look for Buzz C include files
#
find_path(BUZZ_C_INCLUDE_DIR
  NAMES buzz/buzzvm.h
  PATHS ${_BUZZ_C_INCLUDE_PATHS}
  DOC "Location of the Buzz C include files")

#
# Look for Buzz BZZ include files
#
find_path(BUZZ_BZZ_INCLUDE_DIR
  NAMES vec2.bzz
  PATHS ${_BUZZ_BZZ_INCLUDE_PATHS}
  DOC "Location of the Buzz BZZ include files")

# Handle the QUIETLY and REQUIRED arguments and set BUZZ_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BUZZ
  REQUIRED_VARS BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER BUZZ_LIBRARY BUZZ_LIBRARY_DEBUG BUZZ_C_INCLUDE_DIR BUZZ_BZZ_INCLUDE_DIR)

mark_as_advanced(BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER BUZZ_LIBRARY BUZZ_LIBRARY_DEBUG BUZZ_C_INCLUDE_DIR BUZZ_BZZ_INCLUDE_DIR)
