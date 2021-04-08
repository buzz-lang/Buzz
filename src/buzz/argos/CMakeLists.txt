#
# Compile libbuzzargos
#
include_directories(${ARGOS_INCLUDE_DIRS})
link_directories(${ARGOS_LIBRARY_DIR})
link_libraries(${ARGOS_LDFLAGS})
set(ARGOS_BUZZ_SOURCES
  ../buzzlex.h              ../buzzlex.c
  ../buzzparser.h           ../buzzparser.c
  ../buzzasm.h              ../buzzasm.c
  ../buzzdebug.h            ../buzzdebug.c
  buzz_controller.h         buzz_controller.cpp
  buzz_controller_footbot.h buzz_controller_footbot.cpp
  buzz_controller_spiri.h   buzz_controller_spiri.cpp
  buzz_controller_eyebot.h  buzz_controller_eyebot.cpp
  buzz_loop_functions.h     buzz_loop_functions.cpp)
if(ARGOS_QTOPENGL_FOUND)
  set(ARGOS_BUZZ_SOURCES
    ${ARGOS_BUZZ_SOURCES}
    buzz_qt.h buzz_qt.cpp
    buzz_qt_editor.h buzz_qt_editor.cpp
    # buzz_qt_find_dialog.h buzz_qt_find_dialog.cpp
    buzz_qt_main_window.h buzz_qt_main_window.cpp
    buzz_qt_statetree_item.h buzz_qt_statetree_item.cpp
    buzz_qt_statetree_model.h buzz_qt_statetree_model.cpp
    buzz_qt_syntax_highlighter.h buzz_qt_syntax_highlighter.cpp)
endif(ARGOS_QTOPENGL_FOUND)

add_library(argos3plugin_simulator_buzz SHARED ${ARGOS_BUZZ_SOURCES})
target_link_libraries(argos3plugin_simulator_buzz
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  argos3plugin_simulator_footbot
  argos3plugin_simulator_spiri
  buzz)
if(ARGOS_QTOPENGL_FOUND)
  target_link_libraries(argos3plugin_simulator_buzz
    argos3plugin_simulator_qtopengl)
endif(ARGOS_QTOPENGL_FOUND)

install(TARGETS argos3plugin_simulator_buzz LIBRARY DESTINATION ${ARGOS_LIBRARY_DIR})
install(DIRECTORY . DESTINATION include/buzz/argos FILES_MATCHING PATTERN "*.h")

