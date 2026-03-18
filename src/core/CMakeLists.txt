cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# spdlog (for logging)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.3
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(spdlog)

# Core module implementation
add_library(ullfh_core_impl STATIC MarketEvent.cpp)
target_link_libraries(ullfh_core_impl PRIVATE spdlog::spdlog)
