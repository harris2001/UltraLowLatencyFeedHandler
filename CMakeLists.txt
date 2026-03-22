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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -DULLFH_DIAGNOSTICS")
endif()

##################
# Dependencies   #
##################

# spdlog for logging
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.3
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(spdlog)

##################
# Modules        #
##################

function(add_header_only_library name deps)
    add_library(${name} INTERFACE)
    target_include_directories(${name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(${name} INTERFACE cxx_std_20)
    if(deps)
        target_link_libraries(${name} INTERFACE ${deps})
    endif()
endfunction()

add_header_only_library(ullfh_core "spdlog::spdlog")
add_header_only_library(ullfh_network "")
add_header_only_library(ullfh_protocols ullfh_core)
add_header_only_library(ullfh_pipeline ullfh_core)

# Add source subdirectories (Phase 2: header-only, src will be populated in later phases)
# add_subdirectory(src)

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
