cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# Google Benchmark (for performance benchmarking)
if(BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "Disable -Werror for google benchmark" FORCE)

  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(benchmark)

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(benchmark PRIVATE -Wno-c2y-extensions -Wno-error=c2y-extensions)
    target_compile_options(benchmark_main PRIVATE -Wno-c2y-extensions -Wno-error=c2y-extensions)
  endif()

  add_executable(ullfh_benchmarks main.cpp)
  target_link_libraries(ullfh_benchmarks PRIVATE 
    ullfh_core
    ullfh_network
    ullfh_protocols
    ullfh_pipeline
    benchmark::benchmark_main
  )
endif()