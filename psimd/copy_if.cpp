#include <x86intrin.h>
#include <cstdint>
#include "copy_if.h"
#include "asarray.h"
#include "strategy.h"
#include <cstdint>
#include "../contest/test.h"

namespace psimd {

/* static void print_m256(__m256 vec) { */
/*     float arr[8]; */
/*     _mm256_storeu_ps(arr, vec); */
/*     printf("{%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f}\n", */
/*            arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7]); */
/* } */

static void init_avx_lut(__m256i lut[256]) {
    for (int m = 0; m < 256; m++) {
        int idx[8] = {0};
        int k = 0;
        for (int i = 0; i < 8; i++) {
            if ((m >> i) & 1) {
                idx[k++] = i;
            }
        }
        _mm256_store_si256(lut + m, _mm256_setr_epi32(
                idx[0], idx[1], idx[2], idx[3],
                idx[4], idx[5], idx[6], idx[7]));
    }
}

static __m256i avx_lut[256];
static int init_avx_lut_helper = (init_avx_lut(avx_lut), 0);

static size_t copy_if_i32_avx(const int *__restrict src, int *__restrict dst, size_t n) {
    __m256i zero = _mm256_setzero_si256();

    size_t i = 0, j = 0;
    if (((uintptr_t)src & 31) == 0) {
        for (; i + 7 < n; i += 8) {
            __m256i x = _mm256_load_si256((const __m256i *)(src + i));  // Lat 7 CPI 0.5
            __m256i mask = _mm256_cmpgt_epi32(x, zero);     // Lat 1 CPI 1
            int m = _mm256_movemask_ps(_mm256_castsi256_ps(mask));      // Lat 3 CPI 1
            __m256i idx = _mm256_load_si256(avx_lut + m);       // Lat 7 CPI 0.5
            x = _mm256_permutevar8x32_epi32(x, idx);        // Lat 3 CPI 1
            mask = _mm256_permutevar8x32_epi32(mask, idx);  // Lat 3 CPI 1
            _mm256_maskstore_epi32(dst + j, mask, x);       // Lat 3 CPI 1
            j += _mm_popcnt_u32(m);                         // Lat 3 CPI 1
        }
    }
    for (; i < n; i++) {
        if (src[i] > 0) {
            dst[j++] = src[i];
        }
    }
    return j;
}

size_t copy_if<int, strategy::AVX>::operator()(int const *__restrict in, int *__restrict out, size_t size, int predval1, int predval2, binary_predicate_type predtype) const {
    return copy_if_i32_avx(in, out, size);
}

}
