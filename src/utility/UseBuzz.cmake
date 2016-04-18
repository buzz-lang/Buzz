# .rst:
# UseBuzz
# -------
#
# Use Module for Buzz
#
# This file provides functions for Buzz. It is assumed that
# FindBuzz.cmake has already been loaded. See FindBuzz.cmake for
# information on how to load Buzz into your CMake project.
#
# ::
#
#  buzz_make(script.bzz
#            [INCLUDES dep1.bzz [dep2.bzz ...]])
#
# This command compiles script.bzz. If the script depends on other
# files that should trigger re-compilation if modified, the option
# INCLUDES should be used.
#
# The compilation process looks for include files using the path lists
# specified in these variables:
#
# 1. the environment variable BUZZ_INCLUDE_PATH
# 2. the CMake variable BUZZ_INCLUDE_PATH
#
# The Buzz tools are assumed already detected through
# FindBuzz.cmake. However, you can also manually set following the
# CMake variables:
#
# ::
#
#   BUZZ_COMPILER: the full path to bzzc
#   BUZZ_PARSER: the full path to bzzparse
#   BUZZ_ASSEMBLER: the full path to bzzasm
#
# Examples Usages:
#
# ::
#
#   find_package(Buzz)
#   if(BUZZ_FOUND)
#     include(UseBuzz)
#     buzz_make(script1.bzz)
#     buzz_make(script2.bzz INCLUDES inc1.bzz inc2.bzz)
#   endif(BUZZ_FOUND)
#
#   find_package(Buzz REQUIRED)
#   include(UseBuzz)
#   buzz_make(script1.bzz)
#   buzz_make(script2.bzz INCLUDES inc1.bzz inc2.bzz)

#=============================================================================
# Copyright 2016 Carlo Pinciroli <carlo@pinciroli.net>
#=============================================================================

#
# buzz_make() function definition
#
function(buzz_make _SCRIPT)
  # Make sure tool paths have been set
  if("${BUZZ_COMPILER}" STREQUAL "" OR "${BUZZ_PARSER}" STREQUAL "" OR "${BUZZ_ASSEMBLER}" STREQUAL "")
    message(FATAL_ERROR "buzz_make('${_SCRIPT}'): use Find_Package(Buzz) to look for Buzz tools before calling buzz_make().")
  endif("${BUZZ_COMPILER}" STREQUAL "" OR "${BUZZ_PARSER}" STREQUAL "" OR "${BUZZ_ASSEMBLER}" STREQUAL "")
  # Make sure _SCRIPT ends with .bzz
  get_filename_component(_buzz_make_EXT "${_SCRIPT}" EXT)
  if(NOT _buzz_make_EXT STREQUAL ".bzz")
    message(FATAL_ERROR "buzz_make('${_SCRIPT}'): script name must end with .bzz")
  endif(NOT _buzz_make_EXT STREQUAL ".bzz")
  # Make _BYTECODE and _DEBUG name with .bo and .bdb instead of .bzz
  get_filename_component(_buzz_make_FNAME "${_SCRIPT}" NAME_WE)
  set(_buzz_make_BYTECODE "${_buzz_make_FNAME}.bo")
  set(_buzz_make_DEBUG "${_buzz_make_FNAME}.bdb")
  get_filename_component(_buzz_make_DIR "${_SCRIPT}" DIRECTORY)
  if(NOT "${_buzz_make_DIR}" STREQUAL "")
    set(_buzz_make_BYTECODE "${_buzz_make_DIR}${_buzz_make_BYTECODE}")
    set(_buzz_make_DEBUG "${_buzz_make_DIR}${_buzz_make_DEBUG}")
  endif(NOT "${_buzz_make_DIR}" STREQUAL "")
  # Parse function arguments
  cmake_parse_arguments(_buzz_make
    ""             # Options
    ""             # One-value parameters
    "INCLUDES" # Multi-value parameters
    ${ARGN})
  # Compose include path, putting : instead of ;
  set(_buzz_make_BUZZ_INCLUDE_PATH)
  if(NOT $ENV{BUZZ_INCLUDE_PATH} STREQUAL "")
    set(_buzz_make_BUZZ_INCLUDE_PATH "$ENV{BUZZ_INCLUDE_PATH}")
  endif(NOT $ENV{BUZZ_INCLUDE_PATH} STREQUAL "")
  if(NOT ${BUZZ_INCLUDE_PATH} STREQUAL "")
    set(_buzz_make_BUZZ_INCLUDE_PATH "${_buzz_make_BUZZ_INCLUDE_PATH}" "${BUZZ_INCLUDE_PATH}")
  endif(NOT ${BUZZ_INCLUDE_PATH} STREQUAL "")
  string(REPLACE ";" ":" _buzz_make_BUZZ_INCLUDE_PATH "${_buzz_make_BUZZ_INCLUDE_PATH}")
  if(NOT _buzz_make_BUZZ_INCLUDE_PATH STREQUAL "")
    set(_buzz_make_BUZZ_INCLUDE_PATH "-I" "${_buzz_make_BUZZ_INCLUDE_PATH}")
  endif(NOT _buzz_make_BUZZ_INCLUDE_PATH STREQUAL "")
  # Define compilation command
  add_custom_command(
    OUTPUT "${_buzz_make_BYTECODE}" "${_buzz_make_DEBUG}"
    COMMAND "BZZPARSE=${BUZZ_PARSER}" "BZZASM=${BUZZ_ASSEMBLER}" "${BUZZ_COMPILER}" ${_buzz_make_BUZZ_INCLUDE_PATH} -b "${_buzz_make_BYTECODE}" -d "${_buzz_make_DEBUG}" "${CMAKE_CURRENT_SOURCE_DIR}/${_SCRIPT}"
    DEPENDS ${_buzz_make_INCLUDES}
    COMMENT "Compiling Buzz script ${_SCRIPT}")
  # Add target, so compilation is executed
  add_custom_target("${_SCRIPT}" ALL DEPENDS "${_buzz_make_BYTECODE}" "${_buzz_make_DEBUG}")
endfunction(buzz_make)
