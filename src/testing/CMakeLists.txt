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

  buzz_make(test_takeoff_divide_flock.bzz)
  add_dependencies(test_takeoff_divide_flock.bzz bzzparse)
  
  buzz_make(testempty.bzz)
  add_dependencies(testempty.bzz bzzparse)
  
  buzz_make(testif.bzz)
  add_dependencies(testif.bzz bzzparse)
  
  buzz_make(testdebug.bzz)
  add_dependencies(testdebug.bzz bzzparse)
  
  buzz_make(testdocstring.bzz)
  add_dependencies(testdocstring.bzz bzzparse)
  
  buzz_make(testbehavior.bzz)
  add_dependencies(testbehavior.bzz bzzparse)
  
  buzz_make(testclosure.bzz)
  add_dependencies(testclosure.bzz bzzparse)
  
  buzz_make(testhexagon.bzz)
  add_dependencies(testhexagon.bzz bzzparse)
  
  buzz_make(testsquare.bzz)
  add_dependencies(testsquare.bzz bzzparse)
  
  buzz_make(testgradient.bzz)
  add_dependencies(testgradient.bzz bzzparse)
  
  buzz_make(testinclude1.bzz INCLUDES testinclude2.bzz)
  add_dependencies(testinclude1.bzz bzzparse)
  
  buzz_make(testio.bzz)
  add_dependencies(testio.bzz bzzparse)
  
  buzz_make(testleds.bzz)
  add_dependencies(testleds.bzz bzzparse)
  
  buzz_make(testmath.bzz)
  add_dependencies(testmath.bzz bzzparse)
  
  buzz_make(testexpressions.bzz)
  add_dependencies(testexpressions.bzz bzzparse)
  
  buzz_make(testmsg.bzz)
  add_dependencies(testmsg.bzz bzzparse)
  
  buzz_make(testneighbors.bzz)
  add_dependencies(testneighbors.bzz bzzparse)
  
  buzz_make(testparsing.bzz)
  add_dependencies(testparsing.bzz bzzparse)
  
  buzz_make(teststigmergy.bzz)
  add_dependencies(teststigmergy.bzz bzzparse)
  
  buzz_make(teststring.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/string.bzz)
  add_dependencies(teststring.bzz bzzparse)
  
  buzz_make(testswarm.bzz)
  add_dependencies(testswarm.bzz bzzparse)
  
  buzz_make(testtable.bzz)
  add_dependencies(testtable.bzz bzzparse)
  
  buzz_make(testvec2.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/vec2.bzz)
  add_dependencies(testvec2.bzz bzzparse)
  
  buzz_make(testwhile.bzz)
  add_dependencies(testwhile.bzz bzzparse)
  
  buzz_make(testfor.bzz)
  add_dependencies(testfor.bzz bzzparse)
  
  buzz_make(testmobilecode.bzz)
  add_dependencies(testmobilecode.bzz bzzparse)
  
  buzz_make(testbattery.bzz)
  add_dependencies(testbattery.bzz bzzparse)
  
  buzz_make(testtablelib.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/table.bzz)
  add_dependencies(testtablelib.bzz bzzparse)
  
  buzz_make(testneighborsmapreduce.bzz INCLUDES ${CMAKE_SOURCE_DIR}/include/neighbors.bzz ${CMAKE_SOURCE_DIR}/include/table.bzz)
  add_dependencies(testneighborsmapreduce.bzz bzzparse)
  
  buzz_make(testtype.bzz)
  add_dependencies(testtype.bzz bzzparse)  
endif(NOT CMAKE_CROSSCOMPILING)
