add_library(flock SHARED flock.h flock.cpp)
target_link_libraries(flock
  argos3core_simulator
  argos3plugin_simulator_actuators
  argos3plugin_simulator_eyebot
  argos3plugin_simulator_genericrobot
  argos3plugin_simulator_sensors)
