set(_BUZZ_PATHS
  /usr/bin
  /usr/local/bin
  /opt/bin
  /opt/local/bin)

find_program(BUZZ_COMPILER
  NAMES bzzc
  PATHS ${_BUZZ_PATHS}
  DOC "Location of the bzzc compiler")

find_program(BUZZ_PARSER
  NAMES bzzparse
  PATHS ${_BUZZ_PATHS}
  DOC "Location of the bzzparse compiler")

find_program(BUZZ_ASSEMBLER
  NAMES bzzasm
  PATHS ${_BUZZ_PATHS}
  DOC "Location of the bzzasm compiler")

# Handle the QUIETLY and REQUIRED arguments and set BUZZ_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BUZZ
  REQUIRED_VARS BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER)

mark_as_advanced(BUZZ_COMPILER BUZZ_PARSER BUZZ_ASSEMBLER)
