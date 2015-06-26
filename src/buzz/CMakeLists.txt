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
  buzzvm.h buzzvm.c
  buzzasm.h buzzasm.c
  buzzlex.h buzzlex.c
  buzzparser.h buzzparser.c)
target_link_libraries(buzz m)
install(TARGETS buzz LIBRARY DESTINATION lib)

#
# Compile buzzasm
#
add_executable(buzzasm buzzasm_main.c)
target_link_libraries(buzzasm buzz)
install(TARGETS buzzasm RUNTIME DESTINATION bin)

#
# Compile buzzdeasm
#
add_executable(buzzdeasm buzzdeasm_main.c)
target_link_libraries(buzzdeasm buzz)
install(TARGETS buzzdeasm RUNTIME DESTINATION bin)

#
# Compile buzzc
#
add_executable(buzzc buzzc.c)
target_link_libraries(buzzc buzz)
install(TARGETS buzzc RUNTIME DESTINATION bin)
