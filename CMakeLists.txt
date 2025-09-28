cmake_minimum_required(VERSION 3.28)
project(UltraLowLatencyFeedHandler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
FetchContent_Declare(
				googletest
				GIT_REPOSITORY https://github.com/google/googletest.git
				GIT_TAG release-1.17.0
)
FetchContent_MakeAvailable(googletest)

# Google Benchmark (for performance benchmarking)
FetchContent_Declare(
				benchmark
				GIT_REPOSITORY https://github.com/google/benchmark.git
				GIT_TAG main
)
FetchContent_MakeAvailable(benchmark)

