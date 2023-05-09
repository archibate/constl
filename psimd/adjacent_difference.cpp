#include <x86intrin.h>
#include "transpose.h"
#include "asarray.h"
#include <gtest/gtest.h>
#include "../conutils/print.h"

static void adjacent_difference_f32_naive(float const *__restrict in, float *__restrict out, size_t size, float prev) {
    for (size_t i = 0; i < size; i++) {
        float val = in[i];
        out[i] = val - prev;
        prev = val;
    }
}

static void adjacent_difference_i32_naive(int const *__restrict in, int *__restrict out, size_t size, int prev) {
    for (size_t i = 0; i < size; i++) {
        int val = in[i];
        out[i] = val - prev;
        prev = val;
    }
}

static void adjacent_difference_f32_avx(float const *__restrict in, float *__restrict out, size_t size, float prev) {
    __m256 last, next, r, g, b, a, x, y, z, w;
    __m256i perm74560123;
    size_t i;
    last = _mm256_set1_ps(prev);
    perm74560123 = _mm256_setr_epi32(7, 4, 5, 6, 0, 1, 2, 3);
    for (i = 0; i + 31 < size; i += 32) {
        r = _mm256_load_ps(in + i);
        g = _mm256_load_ps(in + i + 8);
        b = _mm256_load_ps(in + i + 16);
        a = _mm256_load_ps(in + i + 24);
        _MM256_TRANSPOSE4_PS(r, g, b, a);
        next = _mm256_permutevar8x32_ps(a, perm74560123);
        x = _mm256_sub_ps(r, _mm256_blend_ps(next, last, 1));
        y = _mm256_sub_ps(g, r);
        z = _mm256_sub_ps(b, g);
        w = _mm256_sub_ps(a, b);
        _MM256_TRANSPOSE4_PS(x, y, z, w);
        _mm256_store_ps(out + i, x);
        _mm256_store_ps(out + i + 8, y);
        _mm256_store_ps(out + i + 16, z);
        _mm256_store_ps(out + i + 24, w);
        last = next;
    }
    _MM_EXTRACT_FLOAT(prev, _mm256_extractf128_ps(last, 0), 0);
    for (; i < size; i++) {
        float val = in[i];
        out[i] = val - prev;
        prev = val;
    }
}

static void adjacent_difference_i32_avx(int const *__restrict in, int *__restrict out, size_t size, int prev) {
    __m256i last, next, r, g, b, a, x, y, z, w, perm74560123;
    size_t i;
    last = _mm256_set1_epi32(prev);
    perm74560123 = _mm256_setr_epi32(7, 4, 5, 6, 0, 1, 2, 3);
    for (i = 0; i + 31 < size; i += 32) {
        r = _mm256_load_si256((__m256i const *)(in + i));
        g = _mm256_load_si256((__m256i const *)(in + i + 8));
        b = _mm256_load_si256((__m256i const *)(in + i + 16));
        a = _mm256_load_si256((__m256i const *)(in + i + 24));
        _MM256_TRANSPOSE4_EPI32(r, g, b, a);
        next = _mm256_permutevar8x32_epi32(a, perm74560123);
        x = _mm256_sub_epi32(r, _mm256_blend_epi32(next, last, 1));
        y = _mm256_sub_epi32(g, r);
        z = _mm256_sub_epi32(b, g);
        w = _mm256_sub_epi32(a, b);
        _MM256_TRANSPOSE4_EPI32(x, y, z, w);
        _mm256_store_si256((__m256i *)(out + i), x);
        _mm256_store_si256((__m256i *)(out + i + 8), y);
        _mm256_store_si256((__m256i *)(out + i + 16), z);
        _mm256_store_si256((__m256i *)(out + i + 24), w);
        last = next;
    }
    prev = _mm256_extract_epi32(last, 0);
    for (; i < size; i++) {
        int val = in[i];
        out[i] = val - prev;
        prev = val;
    }
}

namespace tests {

template <class T>
static void fill_test_data(T *first, T *last, T init) {
    int counter = 1;
    for (T *p = first; p != last; ++p) {
        *p = init;
        init += counter;
        ++counter;
    }
}

class AdjacentDifferenceTestFixture : public ::testing::TestWithParam<int> {
};

INSTANTIATE_TEST_SUITE_P(
    AdjacentDifference,
    AdjacentDifferenceTestFixture,
    ::testing::Values(
        0, 1, 7, 32, 64, 128, 129, 711, 1989, 2013
    ));

TEST_P(AdjacentDifferenceTestFixture, F32AVX) {
    const int size = GetParam();
    alignas(64) float in[size];
    fill_test_data(in, in + size, 3.14f);
    alignas(64) float out[size];

    adjacent_difference_f32_avx(in, out, size, 2.718f);

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            EXPECT_NEAR(out[i], 3.14f - 2.718f, 0.1f);
        } else {
            EXPECT_NEAR(out[i], i, 0.1f);
        }
    }
}

TEST_P(AdjacentDifferenceTestFixture, I32AVX) {
    const int size = GetParam();
    alignas(64) int in[size];
    fill_test_data(in, in + size, 42);
    alignas(64) int out[size];

    adjacent_difference_i32_avx(in, out, size, 4);

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            EXPECT_EQ(out[i], 42 - 4);
        } else {
            EXPECT_EQ(out[i], i);
        }
    }
}

TEST_P(AdjacentDifferenceTestFixture, F32Naive) {
    const int size = GetParam();
    alignas(64) float in[size];
    fill_test_data(in, in + size, 3.14f);
    alignas(64) float out[size];

    adjacent_difference_f32_naive(in, out, size, 2.718f);

    for (int i = 0; i < size; i++) {
        conutils::print(i);
        if (i == 0) {
            EXPECT_NEAR(out[i], 3.14f - 2.718f, 0.1f);
        } else {
            EXPECT_NEAR(out[i], (float)i, 0.1f);
        }
    }
}

TEST_P(AdjacentDifferenceTestFixture, I32Naive) {
    const int size = GetParam();
    alignas(64) int in[size];
    fill_test_data(in, in + size, 42);
    alignas(64) int out[size];

    adjacent_difference_i32_naive(in, out, size, 4);

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            EXPECT_EQ(out[i], 42 - 4);
        } else {
            EXPECT_EQ(out[i], i);
        }
    }
}

}
