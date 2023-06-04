#include "pbb/parallel_for.h"
#include "pbb/parallel_reduce.h"
#include "pbb/parallel_scan.h"
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <vector>
#include <numeric>

TEST(ParallelReduceTest, Sum100000) {
    pbb::begin_arena([&] {
        std::vector<int> values(100000);
        std::iota(values.begin(), values.end(), 1);
        pbb::blocked_range<size_t> range(0, values.size(), 8192);
        auto sum = pbb::parallel_reduce(range, 0, [&] (int &result, pbb::blocked_range<size_t> range) {
            result += std::reduce(values.begin() + range.begin(), values.begin() + range.end(), 0);
        }, [] (int &result, int value) { result += value; });
        EXPECT_EQ(sum, 5000050000); // sum of 1 to 100000
    });
}

/* TEST(ParallelScanTest, Sum100000) { */
/*     pbb::begin_arena([&] { */
/*         std::vector<int> values(100000); */
/*         std::iota(values.begin(), values.end(), 1); */
/*         pbb::blocked_range<size_t> range(0, values.size(), 8192); */
/*         auto sum = pbb::parallel_scan(range, 0, [&] (int &result, pbb::blocked_range<size_t> range) { */
/*             result += std::reduce(values.begin() + range.begin(), values.begin() + range.end(), 0); */
/*         }, [] (int &result, int value) { result += value; }); */
/*         EXPECT_EQ(sum, 5000050000); // sum of 1 to 100000 */
/*     }); */
/* } */
