#
# Install all the files in this directory
#
install(
  DIRECTORY .
  DESTINATION share/buzz/include
  FILES_MATCHING PATTERN "*.bzz")
