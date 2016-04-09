#
# Compile libbuzz
#
add_library(buzz SHARED
  buzzdarray.h buzzdarray.c
  buzzdict.h buzzdict.c
  buzztype.h buzztype.c
  buzzheap.h buzzheap.c
  buzzmsg.h buzzmsg.c
  buzzinmsg.h buzzinmsg.c
  buzzoutmsg.h buzzoutmsg.c
  buzzvstig.h buzzvstig.c
  buzzswarm.h buzzswarm.c
  buzzneighbors.h buzzneighbors.c
  buzzmath.h buzzmath.c
  buzzvm.h buzzvm.c)
target_link_libraries(buzz m)
install(TARGETS buzz LIBRARY DESTINATION lib)
install(DIRECTORY . DESTINATION include/buzz FILES_MATCHING PATTERN "*.h")

#
# Compile buzzasm
#
add_executable(bzzasm buzzdebug.h buzzdebug.c buzzasm.h buzzasm.c buzzasm_main.c)
target_link_libraries(bzzasm buzz)
install(TARGETS bzzasm RUNTIME DESTINATION bin)

#
# Compile buzzdeasm
#
add_executable(bzzdeasm buzzdebug.h buzzdebug.c buzzasm.h buzzasm.c buzzdeasm_main.c)
target_link_libraries(bzzdeasm buzz)
install(TARGETS bzzdeasm RUNTIME DESTINATION bin)

#
# Compile bzzparse
#
add_executable(bzzparse buzzlex.h buzzlex.c buzzparser.h buzzparser.c buzzparse.c)
target_link_libraries(bzzparse buzz)
install(TARGETS bzzparse RUNTIME DESTINATION bin)

#
# Compile ARGoS-related stuff
#
if(ARGOS_FOUND)
  add_subdirectory(argos)
endif(ARGOS_FOUND)
