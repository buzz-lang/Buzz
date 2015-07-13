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
add_executable(buzzasm buzzasm.h buzzasm.c buzzasm_main.c)
target_link_libraries(buzzasm buzz)
install(TARGETS buzzasm RUNTIME DESTINATION bin)

#
# Compile buzzdeasm
#
add_executable(buzzdeasm buzzasm.h buzzasm.c buzzdeasm_main.c)
target_link_libraries(buzzdeasm buzz)
install(TARGETS buzzdeasm RUNTIME DESTINATION bin)

#
# Compile buzzc
#
add_executable(buzzc buzzlex.h buzzlex.c buzzparser.h buzzparser.c buzzc.c)
target_link_libraries(buzzc buzz)
install(TARGETS buzzc RUNTIME DESTINATION bin)

#
# Compile ARGoS-related stuff
#
if(ARGOS_FOUND)
  add_subdirectory(argos)
endif(ARGOS_FOUND)
