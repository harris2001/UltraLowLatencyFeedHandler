# UltraLowLatencyFeedHandler
An ultra-low-latency market data feed handler written in modern C++ for parsing ITCH/FIX protocols

## Features
- High-performance networking with Boost.Asio (TCP/UDP/Multicast)
- Protocol support:
    - ITCH (for order book updates)
    - FIX (for trade messages)
- Low-latency design:
    - Zero-copy buffer management
    - Lock-free message queue
    - Memory pooling for hot paths
- Benchmarks for latency and throughput
- Test suite with Google Test
- Documentation of my design choices

## Requirements
- C++20 or later compiler (GCC, Clang, MSVC)
- CMake 3.16+
- [Asio](https://think-async.com/Asio/) (or Boost.Asio)  
- [spdlog](https://github.com/gabime/spdlog) (logging)
- [GoogleTest](https://github.com/google/googletest) (testing)
- [GoogleBenchmark](https://github.com/google/benchmark) (benchmarking)

## Installation 
```bash
git clone https://github.com/harris2001/UltraLowLatencyFeedHandler.git
cd UltraLowLatencyFeedHandler

./install.sh
```

Note: This code for this repository has been only tested on Linux.
