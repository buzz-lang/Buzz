#
# Compile libbuzzargos
#
add_library(argos3plugin_simulator_buzz SHARED
  buzz_controller.h buzz_controller.cpp
  buzz_controller_footbot.h buzz_controller_footbot.cpp
  buzz_controller_spiri.h buzz_controller_spiri.cpp)
target_link_libraries(argos3plugin_simulator_buzz
  argos3core_simulator
  argos3plugin_simulator_genericrobot
  buzz)
install(TARGETS argos3plugin_simulator_buzz LIBRARY DESTINATION lib/argos3)
