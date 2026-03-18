cmake_minimum_required(VERSION 3.28)

# Protocols module implementation
add_library(ullfh_protocols_impl STATIC IParser.cpp)
target_link_libraries(ullfh_protocols_impl PRIVATE ullfh_core)
