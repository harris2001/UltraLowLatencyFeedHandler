cmake_minimum_required(VERSION 3.28)
project(UltraLowLatencyFeedHandler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

################
# Options      #
################

option(BUILD_TESTS "Enable testing" ON)
option(BUILD_BENCHMARKS "Enable benchmarks" ON)

##################
# Build Presets  #
##################

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -flto")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
endif()

##################
# Modules        #
##################

# Add source subdirectories
add_subdirectory(src/core)
add_subdirectory(src/network)
add_subdirectory(src/protocols)
add_subdirectory(src/pipeline)

# Core module
add_library(ullfh_core INTERFACE)
target_include_directories(ullfh_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/core)
target_compile_features(ullfh_core INTERFACE cxx_std_20)
target_link_libraries(ullfh_core INTERFACE ullfh_core_impl)

# Network module
add_library(ullfh_network INTERFACE)
target_include_directories(ullfh_network INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/network)
target_compile_features(ullfh_network INTERFACE cxx_std_20)
target_link_libraries(ullfh_network INTERFACE ullfh_network_impl)

# Protocols module
add_library(ullfh_protocols INTERFACE)
target_include_directories(ullfh_protocols INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/protocols)
target_compile_features(ullfh_protocols INTERFACE cxx_std_20)
target_link_libraries(ullfh_protocols INTERFACE ullfh_protocols_impl)

# Pipeline module
add_library(ullfh_pipeline INTERFACE)
target_include_directories(ullfh_pipeline INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/pipeline)
target_compile_features(ullfh_pipeline INTERFACE cxx_std_20)
target_link_libraries(ullfh_pipeline INTERFACE ullfh_pipeline_impl)

###############
# Main Target #
###############

add_executable(UltraLowLatencyFeedHandler
  src/main.cpp
)

target_link_libraries(UltraLowLatencyFeedHandler
  PRIVATE ullfh_core
)

target_compile_options(UltraLowLatencyFeedHandler
  PRIVATE -Wall -Wextra -Wpedantic -Werror
)

######################
# Tests & Benchmarks #
######################

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
