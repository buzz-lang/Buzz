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
  FILES bzzc
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

#
# Install the Emacs mode in share/buzz/emacs
#
install(FILES buzz-mode.el DESTINATION share/buzz/emacs)

#
# Install CMake modules in share/buzz/cmake
#
install(FILES FindBuzz.cmake UseBuzz.cmake DESTINATION share/buzz/cmake)

#
# Compress and install man pages in share/man/man1
#
function(man_make _page)
  add_custom_command(
    OUTPUT ${_page}.gz
    COMMAND gzip -c9 ${CMAKE_CURRENT_SOURCE_DIR}/${_page} > ${_page}.gz
    DEPENDS ${_page})
  add_custom_target(${_page} ALL DEPENDS ${_page}.gz)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_page}.gz DESTINATION share/man/man1)
endfunction(man_make _page)
man_make(bzzc.1)
man_make(bzzparse.1)
man_make(bzzasm.1)
man_make(bzzdeasm.1)
man_make(bzzrun.1)
