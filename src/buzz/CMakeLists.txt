#
# Compile libbuzz
#
add_library(buzz SHARED
  buzzdarray.h buzzdarray.c
  buzzdict.h buzzdict.c
  buzzset.h buzzset.c
  buzztype.h buzztype.c
  buzzheap.h buzzheap.c
  buzzmsg.h buzzmsg.c
  buzzinmsg.h buzzinmsg.c
  buzzoutmsg.h buzzoutmsg.c
  buzzvstig.h buzzvstig.c
  buzzswarm.h buzzswarm.c
  buzzneighbors.h buzzneighbors.c
  buzzstrman.h buzzstrman.c
  buzzmath.h buzzmath.c
  buzzio.h buzzio.c
  buzzstring.h buzzstring.c
  buzzvm.h buzzvm.c)
target_link_libraries(buzz m)
install(TARGETS buzz LIBRARY DESTINATION lib)
install(DIRECTORY . DESTINATION include/buzz FILES_MATCHING PATTERN "*.h")

#
# Buzz debugging library
#
add_library(buzzdbg SHARED
  buzzasm.h buzzasm.c 
  buzzdebug.h buzzdebug.c)
target_link_libraries(buzzdbg buzz)
install(TARGETS buzzdbg LIBRARY DESTINATION lib)

#
# Compile bzzasm
#
add_executable(bzzasm buzzasm_main.c)
target_link_libraries(bzzasm buzz buzzdbg)
install(TARGETS bzzasm RUNTIME DESTINATION bin)

#
# Compile bzzdeasm
#
add_executable(bzzdeasm buzzdeasm_main.c)
target_link_libraries(bzzdeasm buzz buzzdbg)
install(TARGETS bzzdeasm RUNTIME DESTINATION bin)

#
# Compile bzzparse
#
add_executable(bzzparse buzzlex.h buzzlex.c buzzparser.h buzzparser.c buzzparse.c)
target_link_libraries(bzzparse buzz)
install(TARGETS bzzparse RUNTIME DESTINATION bin)

#
# Compile bzzrun
#
add_executable(bzzrun buzzrun.c)
target_link_libraries(bzzrun buzz buzzdbg)
install(TARGETS bzzrun RUNTIME DESTINATION bin)

#
# Compile ARGoS-related stuff
#
if(ARGOS_FOUND)
  add_subdirectory(argos)
endif(ARGOS_FOUND)
