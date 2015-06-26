#
# Compile libbuzzargos
#
add_library(buzzargos SHARED
  buzz_controller.h buzz_controller.cpp
  buzz_controller_footbot.h buzz_controller_footbot.cpp
  buzz_controller_spiri.h buzz_controller_spiri.cpp)
target_link_libraries(buzzargos
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  buzz)
