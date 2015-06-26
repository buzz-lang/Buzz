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
install(FILES buzz-mode.el DESTINATION share/buzz)
