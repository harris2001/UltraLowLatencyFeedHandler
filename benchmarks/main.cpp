#include <benchmark/benchmark.h>

static void BM_Stub(benchmark::State& state) {
    for (auto _ : state) {
    }
}
BENCHMARK(BM_Stub);