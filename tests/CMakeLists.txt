cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# Google Test (for unit testing)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(googletest)
  enable_testing()

  add_executable(ullfh_tests main.cpp)
  target_link_libraries(ullfh_tests PRIVATE 
    ullfh_core_impl
    GTest::gtest_main
  )
  add_test(NAME ullfh_tests COMMAND ullfh_tests)
endif()