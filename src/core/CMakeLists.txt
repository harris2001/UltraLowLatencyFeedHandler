cmake_minimum_required(VERSION 3.28)

# Core module implementation
add_library(ullfh_core_impl STATIC MarketEvent.cpp)
target_link_libraries(ullfh_core_impl PRIVATE spdlog::spdlog)
