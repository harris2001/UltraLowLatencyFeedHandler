#!/bin/sh
set -e

BUILD_DIR="build"
NUM_CORES=$(nproc)

echo "Starting build process..."

echo "[-] Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y cmake build-essential libbenchmark-dev

echo "[-] Fetching third-party libraries..."
mkdir -p 3rd_party && cd 3rd_party

[ ! -d "asio" ] && git clone --depth=1 https://github.com/chriskohlhoff/asio.git
[ ! -d "spdlog" ] && git clone --depth=1 https://github.com/gabime/spdlog.git
[ ! -d "Catch2" ] && git clone --depth=1 https://github.com/catchorg/Catch2.git
if [ ! -d "benchmark" ]; then
    git clone --depth=1 https://github.com/google/benchmark.git
    cd benchmark
    git clone --depth=1 https://github.com/google/googletest.git
    cd ..
fi

cd ..

echo ">>> Building project with CMake..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
    -DASIO_INCLUDE_DIR=3rd_party/asio/asio/include \
    -DSPDLOG_DIR=3rd_party/spdlog
cmake --build "$BUILD_DIR" -j"$NUM_CORES"

echo ">>> Running tests..."
ctest --test-dir "$BUILD_DIR" --output-on-failure || true

if [ -f "$BUILD_DIR/benchmarks/benchmark_runner" ]; then
    echo ">>> Running benchmarks..."
    "$BUILD_DIR/benchmarks/benchmark_runner"
else
    echo "[!] No benchmark binary found."
fi
