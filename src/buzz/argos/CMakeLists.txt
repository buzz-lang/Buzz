#
# Compile libbuzzargos
#
include_directories(${ARGOS_INCLUDE_DIRS})
link_directories(${ARGOS_LIBRARY_DIR})
link_libraries(${ARGOS_LDFLAGS})
set(ARGOS_BUZZ_SOURCES
  ../buzzlex.h      ../buzzlex.c
  ../buzzparser.h   ../buzzparser.c
  ../buzzasm.h      ../buzzasm.c
  ../buzzdebug.h    ../buzzdebug.c
  buzz_controller.h buzz_controller.cpp)
if(ARGOS_FOOTBOT_FOUND)
  set(ARGOS_BUZZ_SOURCES ${ARGOS_BUZZ_SOURCES}
    buzz_controller_footbot.h buzz_controller_footbot.cpp)
endif(ARGOS_FOOTBOT_FOUND)
if(ARGOS_SPIRI_FOUND)
  set(ARGOS_BUZZ_SOURCES ${ARGOS_BUZZ_SOURCES}
    buzz_controller_spiri.h buzz_controller_spiri.cpp)
endif(ARGOS_SPIRI_FOUND)
if(ARGOS_EYEBOT_FOUND)
  set(ARGOS_BUZZ_SOURCES ${ARGOS_BUZZ_SOURCES}
    buzz_controller_eyebot.h buzz_controller_eyebot.cpp)
endif(ARGOS_EYEBOT_FOUND)
if(ARGOS_BUILD_FOR STREQUAL "simulator")
  set(ARGOS_BUZZ_SOURCES ${ARGOS_BUZZ_SOURCES}
    buzz_loop_functions.h buzz_loop_functions.cpp)
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
endif(ARGOS_BUILD_FOR STREQUAL "simulator")

add_library(argos3plugin_${ARGOS_BUILD_FOR}_buzz SHARED ${ARGOS_BUZZ_SOURCES})
add_dependencies(argos3plugin_${ARGOS_BUILD_FOR}_buzz buzz)
target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_buzz
  argos3core_${ARGOS_BUILD_FOR}
  argos3plugin_${ARGOS_BUILD_FOR}_genericrobot
  buzz)
if(ARGOS_FOOTBOT_FOUND)
  target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_buzz
    argos3plugin_${ARGOS_BUILD_FOR}_footbot)
endif(ARGOS_FOOTBOT_FOUND)
if(ARGOS_SPIRI_FOUND)
  target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_buzz
    argos3plugin_${ARGOS_BUILD_FOR}_spiri)
endif(ARGOS_SPIRI_FOUND)
if(ARGOS_EYEBOT_FOUND)
  target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_buzz
    argos3plugin_${ARGOS_BUILD_FOR}_eyebot)
endif(ARGOS_EYEBOT_FOUND)
if(ARGOS_QTOPENGL_FOUND)
  target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_buzz
    argos3plugin_${ARGOS_BUILD_FOR}_qtopengl)
endif(ARGOS_QTOPENGL_FOUND)

install(TARGETS argos3plugin_${ARGOS_BUILD_FOR}_buzz LIBRARY DESTINATION ${ARGOS_LIBRARY_DIR})
install(DIRECTORY . DESTINATION include/buzz/argos FILES_MATCHING PATTERN "*.h")
