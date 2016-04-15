#
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
#  make_buzz(script.bzz
#            [DEPENDENCIES dep1.bzz [dep2.bzz ...]])
#
# This command compiles script.bzz. If the script depends on other
# files that should trigger re-compilation if modified, the option
# DEPENDENCIES should be used.
#
# ::
#
# The compilation process looks for include files using the path lists
# specified in these variables:
#
# 1. the environment variable BUZZ_INCLUDE_PATH
# 2. the CMake variable BUZZ_INCLUDE_PATH
#
# ::
#
# The Buzz tools are assumed already detected through
# FindBuzz.cmake. However, you can also manually set following the
# CMake variables:
#
# - BUZZ_COMPILER: the full path to bzzc
# - BUZZ_PARSER: the full path to bzzparse
# - BUZZ_ASSEMBLER: the full path to bzzasm

#=============================================================================
# Copyright 2016 Carlo Pinciroli <carlo@pinciroli.net>
#=============================================================================

#
# make_buzz() function definition
#
function(make_buzz _SCRIPT)
  # Make sure tool paths have been set
  if("${BUZZ_COMPILER}" STREQUAL "" OR "${BUZZ_PARSER}" STREQUAL "" OR "${BUZZ_ASSEMBLER}" STREQUAL "")
    message(FATAL_ERROR "make_buzz('${_SCRIPT}'): use Find_Package(Buzz) to look for Buzz tools before calling make_buzz().")
  endif(${BUZZ_COMPILER} STREQUAL "" OR ${BUZZ_PARSER} STREQUAL "" OR ${BUZZ_ASSEMBLER} STREQUAL "")
  # Make sure _SCRIPT ends with .bzz
  get_filename_component(_make_buzz_EXT "${_SCRIPT}" EXT)
  if(NOT _make_buzz_EXT STREQUAL ".bzz")
    message(FATAL_ERROR "make_buzz('${_SCRIPT}'): script name must end with .bzz")
  endif(NOT _make_buzz_EXT STREQUAL ".bzz")
  # Make _BYTECODE and _DEBUG name with .bo and .bdb instead of .bzz
  get_filename_component(_make_buzz_DIR "${_SCRIPT}" DIRECTORY)
  get_filename_component(_make_buzz_FNAME "${_SCRIPT}" NAME_WE)
  string(CONCAT _make_buzz_BYTECODE "${_make_buzz_DIR}" "${_make_buzz_FNAME}" ".bo")
  string(CONCAT _make_buzz_DEBUG "${_make_buzz_DIR}" "${_make_buzz_FNAME}" ".bdb")
  # Parse function arguments
  cmake_parse_arguments(_make_buzz
    ""             # Options
    ""             # One-value parameters
    "DEPENDENCIES" # Multi-value parameters
    ${ARGN})
  # Compose include path, putting : instead of ;
  set(_make_buzz_BUZZ_INCLUDE_PATH)
  if(NOT $ENV{BUZZ_INCLUDE_PATH} STREQUAL "")
    set(_make_buzz_BUZZ_INCLUDE_PATH "$ENV{BUZZ_INCLUDE_PATH}")
  endif(NOT $ENV{BUZZ_INCLUDE_PATH} STREQUAL "")
  if(NOT ${BUZZ_INCLUDE_PATH} STREQUAL "")
    set(_make_buzz_BUZZ_INCLUDE_PATH "${_make_buzz_BUZZ_INCLUDE_PATH}" "${BUZZ_INCLUDE_PATH}")
  endif(NOT ${BUZZ_INCLUDE_PATH} STREQUAL "")
  string(REPLACE ";" ":" _make_buzz_BUZZ_INCLUDE_PATH "${_make_buzz_BUZZ_INCLUDE_PATH}")
  if(NOT _make_buzz_BUZZ_INCLUDE_PATH STREQUAL "")
    set(_make_buzz_BUZZ_INCLUDE_PATH "-I" "${_make_buzz_BUZZ_INCLUDE_PATH}")
  endif(NOT _make_buzz_BUZZ_INCLUDE_PATH STREQUAL "")
  # Define compilation command
  add_custom_command(
    OUTPUT ${_make_buzz_BYTECODE} ${_make_buzz_DEBUG}
    COMMAND "BZZPARSE=${BUZZ_PARSER}" "BZZASM=${BUZZ_ASSEMBLER}" "${BUZZ_COMPILER}" ${_make_buzz_BUZZ_INCLUDE_PATH} -b "${_make_buzz_BYTECODE}" -d "${_make_buzz_DEBUG}" "${CMAKE_CURRENT_SOURCE_DIR}/${_SCRIPT}"
    DEPENDS ${_make_buzz_DEPENDENCIES}
    COMMENT "Building Buzz script ${_SCRIPT}")
  # Add target, so compilation is executed
  add_custom_target(${_SCRIPT} ALL DEPENDS ${_make_buzz_BYTECODE} ${_make_buzz_DEBUG})
endfunction(make_buzz)
