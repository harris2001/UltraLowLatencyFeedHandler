cmake_minimum_required(VERSION 3.28)

# Pipeline module implementation
add_library(ullfh_pipeline_impl STATIC SpscQueue.cpp)
target_link_libraries(ullfh_pipeline_impl PRIVATE ullfh_core)
