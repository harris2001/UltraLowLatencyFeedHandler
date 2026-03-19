cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# Google Benchmark (for performance benchmarking)
if(BUILD_BENCHMARKS)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(benchmark)

  add_executable(ullfh_benchmarks main.cpp)
  target_link_libraries(ullfh_benchmarks PRIVATE 
    ullfh_core
    ullfh_network
    ullfh_protocols
    ullfh_pipeline
    benchmark::benchmark_main
  )
endif()