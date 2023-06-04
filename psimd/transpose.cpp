#include <x86intrin.h>
#include "transpose.h"
#include "asarray.h"
#include "../contest/test.h"

namespace psimd {

namespace testing {
namespace {

TEST(Transpose_MM256Transpose4PS) {
    __m256 row0 = _mm256_setr_ps(1, 2, 3, 4, 5, 6, 7, 8);
    __m256 row1 = _mm256_setr_ps(9, 10, 11, 12, 13, 14, 15, 16);
    __m256 row2 = _mm256_setr_ps(17, 18, 19, 20, 21, 22, 23, 24);
    __m256 row3 = _mm256_setr_ps(25, 26, 27, 28, 29, 30, 31, 32);

    _MM256_TRANSPOSE4_PS(row0, row1, row2, row3);

    __m256 expected_row0 = _mm256_setr_ps(1, 9, 17, 25, 5, 13, 21, 29);
    __m256 expected_row1 = _mm256_setr_ps(2, 10, 18, 26, 6, 14, 22, 30);
    __m256 expected_row2 = _mm256_setr_ps(3, 11, 19, 27, 7, 15, 23, 31);
    __m256 expected_row3 = _mm256_setr_ps(4, 12, 20, 28, 8, 16, 24, 32);

    EXPECT_EQ(asarray(row0), asarray(expected_row0));
    EXPECT_EQ(asarray(row1), asarray(expected_row1));
    EXPECT_EQ(asarray(row2), asarray(expected_row2));
    EXPECT_EQ(asarray(row3), asarray(expected_row3));
}

TEST(Transpose_MM256Transpose4EPI32) {
    __m256i row0 = _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    __m256i row1 = _mm256_setr_epi32(9, 10, 11, 12, 13, 14, 15, 16);
    __m256i row2 = _mm256_setr_epi32(17, 18, 19, 20, 21, 22, 23, 24);
    __m256i row3 = _mm256_setr_epi32(25, 26, 27, 28, 29, 30, 31, 32);

    _MM256_TRANSPOSE4_EPI32(row0, row1, row2, row3);

    __m256i expected_row0 = _mm256_setr_epi32(1, 9, 17, 25, 5, 13, 21, 29);
    __m256i expected_row1 = _mm256_setr_epi32(2, 10, 18, 26, 6, 14, 22, 30);
    __m256i expected_row2 = _mm256_setr_epi32(3, 11, 19, 27, 7, 15, 23, 31);
    __m256i expected_row3 = _mm256_setr_epi32(4, 12, 20, 28, 8, 16, 24, 32);

    EXPECT_EQ(asarray(row0), asarray(expected_row0));
    EXPECT_EQ(asarray(row1), asarray(expected_row1));
    EXPECT_EQ(asarray(row2), asarray(expected_row2));
    EXPECT_EQ(asarray(row3), asarray(expected_row3));
}

}
}

}
