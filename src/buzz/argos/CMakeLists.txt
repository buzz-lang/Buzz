#
# Compile libbuzzargos
#
set(ARGOS_BUZZ_SOURCES
  ../buzzasm.h ../buzzasm.c
  ../buzzdebug.h ../buzzdebug.c
  buzz_controller.h buzz_controller.cpp
  buzz_controller_footbot.h buzz_controller_footbot.cpp
  buzz_controller_spiri.h buzz_controller_spiri.cpp)
if(ARGOS_COMPILE_QTOPENGL)
  set(ARGOS_BUZZ_SOURCES
    ${ARGOS_BUZZ_SOURCES}
    buzz_qt.h buzz_qt.cpp)
endif(ARGOS_COMPILE_QTOPENGL)

add_library(argos3plugin_simulator_buzz SHARED ${ARGOS_BUZZ_SOURCES})
target_link_libraries(argos3plugin_simulator_buzz
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  argos3plugin_simulator_footbot
  argos3plugin_simulator_spiri
  buzz)
if(ARGOS_COMPILE_QTOPENGL)
  target_link_libraries(argos3plugin_simulator_buzz
    argos3plugin_simulator_qtopengl
    ${QT_LIBRARIES} ${GLUT_LIBRARY} ${OPENGL_LIBRARY})
endif(ARGOS_COMPILE_QTOPENGL)

install(TARGETS argos3plugin_simulator_buzz LIBRARY DESTINATION ${ARGOS_PREFIX}/lib/argos3)
install(DIRECTORY . DESTINATION include/buzz/argos FILES_MATCHING PATTERN "*.h")

