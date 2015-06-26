#
# Get information about the current processor
#
execute_process(
  COMMAND uname -m
  COMMAND tr -d '\n'
  OUTPUT_VARIABLE BUZZ_PROCESSOR_ARCH)

#
# General compilation flags
#
set(CMAKE_C_FLAGS   "-Wall -std=c99")
set(CMAKE_CXX_FLAGS "-Wall")
if(NOT APPLE)
  set(BUZZ_FLAGS_DEBUG "-ggdb3")
  add_definitions(-D_GNU_SOURCE)
else(NOT APPLE)
  set(BUZZ_FLAGS_DEBUG "-gdwarf-4")
endif(NOT APPLE)
set(CMAKE_C_FLAGS_DEBUG             ${BUZZ_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_RELEASE           "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${BUZZ_FLAGS_DEBUG} -Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG           ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE         ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "-ggdb3")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "-ggdb3")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "-ggdb3")
