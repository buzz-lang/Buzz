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

add_executable(testbuzz ../buzz/buzzasm.h ../buzz/buzzasm.c ../buzz/buzzdebug.h ../buzz/buzzdebug.c testbuzz.c)
target_link_libraries(testbuzz buzz)

#
# Test scripts
#

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/utility)
set(BUZZ_COMPILER ${CMAKE_SOURCE_DIR}/utility/bzzc)
set(BUZZ_PARSER ${CMAKE_BINARY_DIR}/buzz/bzzparse)
set(BUZZ_ASSEMBLER ${CMAKE_BINARY_DIR}/buzz/bzzasm)
include(UseBuzz)
set(BUZZ_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

make_buzz(test_takeoff_divide_flock.bzz)
make_buzz(testbehavior.bzz)
make_buzz(testclosure.bzz)
make_buzz(testflocking.bzz)
make_buzz(testgradient.bzz)
make_buzz(testinclude1.bzz)
make_buzz(testinclude2.bzz)
make_buzz(testmath.bzz)
make_buzz(testneighbors.bzz)
make_buzz(testparsing.bzz)
make_buzz(teststigmergy.bzz)
make_buzz(testswarm.bzz)
make_buzz(testtable.bzz)
make_buzz(testwhile.bzz)
