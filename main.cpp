#if 0
#include "radix_sort.h"
#include <benchmark/benchmark.h>
static void BM_radix_sort(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (size_t i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        radix_sort(pos.data(), (size_t)m);
    }
}
#if 1
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <vector>
#include "radix_sort.h"
#include <benchmark/benchmark.h>

constexpr int n = 10'000'000;
constexpr int m = n * 2;

static void BM_atomic_count_relaxed_radix_sorted(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (size_t i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    radix_sort(pos.data(), (size_t)m);
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        #pragma omp parallel for
        for (size_t i = 0; i < m; i++) {
            std::atomic_ref(cnt[pos[i]]).fetch_add(1, std::memory_order_relaxed);
        }
    }
}
/* BENCHMARK(BM_atomic_count_relaxed_radix_sorted); */

static void BM_atomic_count_relaxed_sorted(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (size_t i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    std::sort(pos.begin(), pos.end());
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        #pragma omp parallel for
        for (size_t i = 0; i < m; i++) {
            std::atomic_ref(cnt[pos[i]]).fetch_add(1, std::memory_order_relaxed);
        }
    }
}
BENCHMARK(BM_atomic_count_relaxed_sorted);

static void BM_atomic_count_seq_cst(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (size_t i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        #pragma omp parallel for
        for (size_t i = 0; i < m; i++) {
            std::atomic_ref(cnt[pos[i]]).fetch_add(1);
        }
    }
}
BENCHMARK(BM_atomic_count_seq_cst);

static void BM_atomic_count_sequential(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (size_t i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        for (size_t i = 0; i < m; i++) {
            ++cnt[pos[i]];
        }
    }
}
BENCHMARK(BM_atomic_count_sequential);

static void BM_atomic_count_relaxed(benchmark::State &state) {
    std::vector<uint32_t> pos(m);
    for (int i = 0; i < m; i++) {
        pos[i] = rand() % n;
    }
    std::vector<uint32_t> cnt(n);
    for (auto _ : state) {
        #pragma omp parallel for
        for (int i = 0; i < m; i++) {
            std::atomic_ref(cnt[pos[i]]).fetch_add(1, std::memory_order_relaxed);
        }
    }
}
BENCHMARK(BM_atomic_count_relaxed);

BENCHMARK_MAIN();
#else
#include "radix_sort.h"
#include <gtest/gtest.h>

TEST(RadixSortTest, SortsInAscendingOrder) {
  uint32_t input[] = {5, 2, 7, 3, 1};
  uint32_t expected[] = {1, 2, 3, 5, 7};
  const size_t size = sizeof(input) / sizeof(uint32_t);

  radix_sort(input, size);

  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(input[i], expected[i]);
  }
}
#endif
#endif
