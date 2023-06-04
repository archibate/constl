#pragma once

#include <immintrin.h>
#include <cmath>

namespace consimd {

static __m128 mm_sincos_ps(__m128 xx, __m128 *cosret) {
    const __m128 DP1F = _mm_set1_ps(0.78515625f * 2.f);
    const __m128 DP2F = _mm_set1_ps(2.4187564849853515625E-4f * 2.f);
    const __m128 DP3F = _mm_set1_ps(3.77489497744594108E-8f * 2.f);
    const __m128 P0sinf = _mm_set1_ps(-1.6666654611E-1f);
    const __m128 P1sinf = _mm_set1_ps(8.3321608736E-3f);
    const __m128 P2sinf = _mm_set1_ps(-1.9515295891E-4f);
    const __m128 P0cosf = _mm_set1_ps(4.166664568298827E-2f);
    const __m128 P1cosf = _mm_set1_ps(-1.388731625493765E-3f);
    const __m128 P2cosf = _mm_set1_ps(2.443315711809948E-5f);

    __m128 xa = _mm_and_ps(xx, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
    __m128i q = _mm_cvtps_epi32(_mm_mul_ps(xa, _mm_set1_ps(2.f / M_PI)));
    __m128 y = _mm_cvtepi32_ps(q);
    __m128 x = _mm_fnmadd_ps(y, DP3F, _mm_fnmadd_ps(y, DP2F, _mm_fnmadd_ps(y, DP1F, xa)));
    __m128 x2 = _mm_mul_ps(x, x);
    __m128 x3 = _mm_mul_ps(x2, x);
    __m128 x4 = _mm_mul_ps(x2, x2);
    __m128 s = _mm_fmadd_ps(x3, _mm_fmadd_ps(x2, P2sinf, _mm_fmadd_ps(x, P1sinf, P0sinf)), x);
    __m128 c = _mm_fmadd_ps(x4, _mm_fmadd_ps(x2, P2cosf, _mm_fmadd_ps(x, P1cosf, P0cosf)), _mm_fnmadd_ps(_mm_set1_ps(0.5f), x2, _mm_set1_ps(1.0f)));
    __m128 mask = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(q), _mm_castsi128_ps(_mm_set1_epi32(0x1))))));
    __m128 sin1 = _mm_blendv_ps(c, s, mask);
    __m128 cos1 = _mm_blendv_ps(s, c, mask);
    /* __m128 sin1 = _mm_or_ps(_mm_and_ps(c, swap), _mm_andnot_ps(s, swap)); */
    /* __m128 cos1 = _mm_or_ps(_mm_and_ps(s, swap), _mm_andnot_ps(c, swap)); */
    sin1 = _mm_xor_ps(sin1, _mm_and_ps(_mm_xor_ps(_mm_castsi128_ps(_mm_slli_epi32(q, 30)), xx), _mm_set1_ps(-0.0f)));
    cos1 = _mm_xor_ps(cos1, _mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(_mm_add_epi32(q, _mm_set1_epi32(1))), _mm_castsi128_ps(_mm_set1_epi32(0x2)))), 30)));
    *cosret = cos1;
    return sin1;
}

}
