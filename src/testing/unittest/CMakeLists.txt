#
# Google Test
#

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  buzztype_test
  buzztype_test.cpp
)
target_link_libraries(
  buzztype_test
  gtest_main
  buzz
)

include(GoogleTest)
gtest_discover_tests(buzztype_test)
