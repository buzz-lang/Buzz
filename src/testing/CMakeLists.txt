#
# Test programs
#

add_executable(testbuzzdarray testbuzzdarray.c)
target_link_libraries(testbuzzdarray buzz)

add_executable(testbuzzdict testbuzzdict.c)
target_link_libraries(testbuzzdict buzz)

add_executable(testbuzzset testbuzzset.c)
target_link_libraries(testbuzzset buzz)

add_executable(testbuzzlex ../buzz/buzzlex.h ../buzz/buzzlex.c testbuzzlex.c)
target_link_libraries(testbuzzlex buzz)

add_executable(testbuzzstrman testbuzzstrman.c)
target_link_libraries(testbuzzstrman buzz)

if(ARGOS_FOUND)
  include_directories(${ARGOS_INCLUDE_DIRS})
  link_directories(${ARGOS_LIBRARY_DIR})
  link_libraries(${ARGOS_LDFLAGS})
  add_library(testloopfunctions MODULE testloopfunctions.h testloopfunctions.cpp)
  target_link_libraries(testloopfunctions argos3plugin_simulator_buzz buzz argos3core_simulator)
endif(ARGOS_FOUND)

#
# Test scripts
#

function(_buzz_make_test _script)
  if(ARGN)
    if(NOT ARGV1 STREQUAL "INCLUDES")
      message(FATAL_ERROR "Expected INCLUDES in _buzz_make_test(script INCLUDES ...), got ${ARGV1}")
    endif(NOT ARGV1 STREQUAL "INCLUDES")
    if(ARGC LESS 3)
      message(FATAL_ERROR "Expected scripts to include in _buzz_make_test(script INCLUDES ...)")
    endif(ARGC LESS 3)
    buzz_make(${_script} ${ARGN})
  else(ARGN)
    buzz_make(${_script})
  endif(ARGN)
  add_dependencies(${_script} bzzasm bzzdeasm bzzparse)
endfunction(_buzz_make_test)

if(NOT CMAKE_CROSSCOMPILING)
  # Make sure only the locally compiled tools are used
  set(BUZZ_COMPILER ${CMAKE_BINARY_DIR}/utility/bzzc)
  set(BUZZ_PARSER ${CMAKE_BINARY_DIR}/buzz/bzzparse)
  set(BUZZ_ASSEMBLER ${CMAKE_BINARY_DIR}/buzz/bzzasm)
  set(BUZZ_BZZ_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR})
  # Initialize Buzz CMake stuff
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/utility)
  include(UseBuzz)

  _buzz_make_test(test_takeoff_divide_flock.bzz)
  _buzz_make_test(testempty.bzz)
  _buzz_make_test(testif.bzz)
  _buzz_make_test(testdebug.bzz)
  _buzz_make_test(testdocstring.bzz)
  _buzz_make_test(testbehavior.bzz)
  _buzz_make_test(testclosure.bzz)
  _buzz_make_test(testhexagon.bzz)
  _buzz_make_test(testsquare.bzz)
  _buzz_make_test(testgradient.bzz)
  _buzz_make_test(testinclude1.bzz INCLUDES testinclude2.bzz)
  _buzz_make_test(testio.bzz)
  _buzz_make_test(testleds.bzz)
  _buzz_make_test(testmath.bzz)
  _buzz_make_test(testexpressions.bzz)
  _buzz_make_test(testmsg.bzz)
  _buzz_make_test(testneighbors.bzz)
  _buzz_make_test(testparsing.bzz)
  _buzz_make_test(teststigmergy.bzz)
  _buzz_make_test(teststring.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/string.bzz)
  _buzz_make_test(testswarm.bzz)
  _buzz_make_test(testtable.bzz)
  _buzz_make_test(testvec2.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/vec2.bzz)
  _buzz_make_test(testwhile.bzz)
  _buzz_make_test(testfor.bzz)
  _buzz_make_test(testmobilecode.bzz)
  _buzz_make_test(testbattery.bzz)
  _buzz_make_test(testtablelib.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/table.bzz)
  _buzz_make_test(testneighborsmapreduce.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/neighbors.bzz ${CMAKE_SOURCE_DIR}/include/table.bzz)
  _buzz_make_test(testtype.bzz)
  _buzz_make_test(testmatrix.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/matrix.bzz)
endif(NOT CMAKE_CROSSCOMPILING)
