#
# Test programs
#

add_executable(testbuzzdarray testbuzzdarray.c)
target_link_libraries(testbuzzdarray buzz)

add_executable(testbuzzdict testbuzzdict.c)
target_link_libraries(testbuzzdict buzz)

add_executable(testbuzzmsg testbuzzmsg.c)
target_link_libraries(testbuzzmsg buzz)

add_executable(testbuzzlex ../buzz/buzzlex.h ../buzz/buzzlex.c testbuzzlex.c)
target_link_libraries(testbuzzlex buzz)

add_executable(testbuzzstrman testbuzzstrman.c)
target_link_libraries(testbuzzstrman buzz)

#
# Test scripts
#

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/utility)
set(BUZZ_COMPILER ${CMAKE_SOURCE_DIR}/utility/bzzc)
set(BUZZ_PARSER ${CMAKE_BINARY_DIR}/buzz/bzzparse)
set(BUZZ_ASSEMBLER ${CMAKE_BINARY_DIR}/buzz/bzzasm)
include(UseBuzz)
set(BUZZ_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

buzz_make(test_takeoff_divide_flock.bzz)
buzz_make(testbehavior.bzz)
buzz_make(testclosure.bzz)
buzz_make(testflocking.bzz)
buzz_make(testgradient.bzz)
buzz_make(testinclude1.bzz INCLUDES testinclude2.bzz)
buzz_make(testio.bzz)
buzz_make(testmath.bzz)
buzz_make(testneighbors.bzz)
buzz_make(testparsing.bzz)
buzz_make(teststigmergy.bzz)
buzz_make(teststring.bzz)
buzz_make(testswarm.bzz)
buzz_make(testtable.bzz)
buzz_make(testwhile.bzz)
