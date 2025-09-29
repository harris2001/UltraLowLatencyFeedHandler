cmake_minimum_required(VERSION 3.28)
project(UltraLowLatencyFeedHandler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

################
# Dependencies #
################

# Asio (for networking)
FetchContent_Declare(
	asio
	GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
	GIT_TAG master
)
FetchContent_MakeAvailable(asio)

# spdlog (for logging)
FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.15.3 
)
FetchContent_MakeAvailable(spdlog)

# Google Test (for unit testing)
option(BUILD_TESTS "Enable testing" ON)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
	# Prevent GoogleTest from overriding our compiler/linker options
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)

  enable_testing()
endif()

# Google Benchmark (for performance benchmarking)
option(BUILD_BENCHMARKS "Enable benchmarks" ON)
if(BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
  	benchmark
  	GIT_REPOSITORY https://github.com/google/benchmark.git
  	GIT_TAG main
  )
  FetchContent_MakeAvailable(benchmark)
endif()

###############
# Main Target #
###############

add_executable(UltraLowLatencyFeedHandler
  src/main.cpp
)

target_link_libraries(UltraLowLatencyFeedHandler
  PRIVATE asio
  PRIVATE spdlog::spdlog
)

target_compile_options(UltraLowLatencyFeedHandler
  PRIVATE -Wall -Wextra -Wpedantic -Werror
)
