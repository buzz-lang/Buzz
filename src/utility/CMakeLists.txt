#
# Configuration file for pkg-config
#
configure_file(
  buzz.pc.in
  ${CMAKE_BINARY_DIR}/utility/buzz.pc
  @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/utility/buzz.pc DESTINATION lib/pkgconfig)

#
# Uninstall script
#
configure_file(
  uninstall.sh.in
  ${CMAKE_BINARY_DIR}/utility/uninstall.sh
  @ONLY)
install(
  FILES ${CMAKE_BINARY_DIR}/utility/uninstall.sh
  DESTINATION share/buzz
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

#
# Install bzzcompile in bin/
#
install(
  FILES bzzcompile
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

#
# Install the Emacs mode in share/buzz
#
install(FILES buzz-mode.el DESTINATION share/buzz/emacs)
