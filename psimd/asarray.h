#pragma once

#include <x86intrin.h>
#include <array>

inline std::array<float, 8> asarray(__m256 m) {
    alignas(32) std::array<float, 8> arr;
    _mm256_store_ps(arr.data(), m);
    return arr;
}

inline std::array<int, 8> asarray(__m256i m) {
    alignas(32) std::array<int, 8> arr;
    _mm256_store_si256((__m256i *)arr.data(), m);
    return arr;
}

inline std::array<float, 4> asarray(__m128 m) {
    alignas(16) std::array<float, 4> arr;
    _mm_store_ps(arr.data(), m);
    return arr;
}

inline std::array<int, 4> asarray(__m128i m) {
    alignas(16) std::array<int, 4> arr;
    _mm_store_epi32(arr.data(), m);
    return arr;
}
