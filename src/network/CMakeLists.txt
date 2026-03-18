cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# Asio (for networking)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-28-0
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(asio)

# Network module implementation
add_library(ullfh_network_impl STATIC UdpReceiver.cpp)
target_link_libraries(ullfh_network_impl PRIVATE asio)
