#pragma once

#include <numeric>
#include <algorithm>
#include <array>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_scan.h>

// https://zhuanlan.zhihu.com/p/491798194
static void radix_sort(uint32_t *in, uint32_t *out, size_t in_size) {
    constexpr uint32_t W = 8;
    uint32_t indices[1 << W];
    for (uint32_t d = 0; d < 32; d += W) {
        struct ArrayReducer {
            explicit ArrayReducer(uint32_t *in_, uint32_t d_)
            : in(in_), d(d_) {
                std::fill_n(counts, 1 << W, 0);
            }

            ArrayReducer(ArrayReducer &other, tbb::split)
            : in(other.in) {
                std::copy_n(other.counts, 1 << W, counts);
            }

            void operator()(tbb::blocked_range<size_t> const &range) {
                for (size_t i = range.begin(); i < range.end(); i++) {
                    uint32_t val = in[i];
                    uint32_t bid = (val >> d) & ((1 << W) - 1);
                    ++counts[bid];
                }
            }

            void join(ArrayReducer &other) {
                std::transform(counts, counts + (1 << W), other.counts, counts, std::plus<uint32_t>());
            }

            uint32_t *in;
            uint32_t d;
            uint32_t counts[1 << W];
        };
        ArrayReducer reducer(in, d);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, in_size), reducer);
        std::exclusive_scan(reducer.counts, reducer.counts + (1 << W), indices, uint32_t(0), std::plus<uint32_t>());
        struct ArrayEmitter {
            explicit ArrayEmitter(uint32_t *in_, uint32_t *out_, uint32_t d_, uint32_t *indices_)
            : in(in_), out(out_), d(d_), indices(indices_) {
            }

            void operator()(tbb::blocked_range<size_t> const &range) const {
                for (size_t i = range.begin(); i < range.end(); i++) {
                    uint32_t val = in[i];
                    uint32_t bid = (val >> d) & ((1 << W) - 1);
                    out[indices[bid]++] = val;
                }
            }

            uint32_t *in;
            uint32_t *out;
            uint32_t d;
            uint32_t *indices;
        };
        ArrayEmitter emitter(in, out, d, indices);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, in_size), emitter);
        std::swap(in, out);
    }
}

static void radix_sort(uint32_t *in, size_t in_size) {
    std::vector<uint32_t> out(in_size);
    radix_sort(in, out.data(), in_size);
    std::copy_n(out.data(), in_size, in);
}
