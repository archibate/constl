#pragma once

#include <cstddef>
#include "strategy.h"

namespace psimd {

template <class T, class Strategy>
struct adjacent_difference {
    void operator()(T const *__restrict in, T *__restrict out, size_t size, T prev) const {
        for (size_t i = 0; i < size; i++) {
            T val = in[i];
            out[i] = val - prev;
            prev = val;
        }
    }
};

template <>
struct adjacent_difference<float, strategy::AVX> {
    void operator()(float const *__restrict in, float *__restrict out, size_t size, float prev) const;
};

template <>
struct adjacent_difference<int, strategy::AVX> {
    void operator()(int const *__restrict in, int *__restrict out, size_t size, int prev) const;
};
}

