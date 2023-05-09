#pragma once

#include <x86intrin.h>


#define _MM256_TRANSPOSE4_PS(row0, row1, row2, row3) \
{ \
    __m256 tmp0, tmp1, tmp2, tmp3; \
    tmp0 = _mm256_unpacklo_ps(row0, row1); \
    tmp1 = _mm256_unpackhi_ps(row0, row1); \
    tmp2 = _mm256_unpacklo_ps(row2, row3); \
    tmp3 = _mm256_unpackhi_ps(row2, row3); \
    row0 = _mm256_shuffle_ps(tmp0, tmp2, _MM_SHUFFLE(1,0,1,0)); \
    row1 = _mm256_shuffle_ps(tmp0, tmp2, _MM_SHUFFLE(3,2,3,2)); \
    row2 = _mm256_shuffle_ps(tmp1, tmp3, _MM_SHUFFLE(1,0,1,0)); \
    row3 = _mm256_shuffle_ps(tmp1, tmp3, _MM_SHUFFLE(3,2,3,2)); \
}

#define _MM256_TRANSPOSE4_EPI32(row0, row1, row2, row3) \
{ \
    __m256 tmp0, tmp1, tmp2, tmp3; \
    tmp0 = _mm256_castsi256_ps(_mm256_unpacklo_epi32(row0, row1)); \
    tmp1 = _mm256_castsi256_ps(_mm256_unpackhi_epi32(row0, row1)); \
    tmp2 = _mm256_castsi256_ps(_mm256_unpacklo_epi32(row2, row3)); \
    tmp3 = _mm256_castsi256_ps(_mm256_unpackhi_epi32(row2, row3)); \
    row0 = _mm256_castps_si256(_mm256_shuffle_ps(tmp0, tmp2, _MM_SHUFFLE(1,0,1,0))); \
    row1 = _mm256_castps_si256(_mm256_shuffle_ps(tmp0, tmp2, _MM_SHUFFLE(3,2,3,2))); \
    row2 = _mm256_castps_si256(_mm256_shuffle_ps(tmp1, tmp3, _MM_SHUFFLE(1,0,1,0))); \
    row3 = _mm256_castps_si256(_mm256_shuffle_ps(tmp1, tmp3, _MM_SHUFFLE(3,2,3,2))); \
}

inline __m256 mm256_swap_low_high_ps(__m256 v) {
    return _mm256_permutevar8x32_ps(v, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
}

inline __m256i mm256_swap_low_high_epi32(__m256i v) {
    return _mm256_permutevar8x32_epi32(v, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
}
