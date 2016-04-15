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
# Standard Buzz include paths
#
set(_BUZZ_INCLUDE_PATHS
  /usr/include
  /usr/local/include
  /opt/include
  /opt/local/include)

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
# Look for Buzz include files
#
find_path(BUZZ_INCLUDE_DIR
  NAMES buzz/buzzvm.h
  PATHS ${_BUZZ_INCLUDE_PATHS}
  DOC "Location of the Buzz include files")

# Handle the QUIETLY and REQUIRED arguments and set BUZZ_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BUZZ
  REQUIRED_VARS BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER BUZZ_LIBRARY BUZZ_INCLUDE_DIR)

mark_as_advanced(BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER BUZZ_LIBRARY BUZZ_INCLUDE_DIR)
