#pragma once

#include <cstddef>
#include "strategy.h"

namespace psimd {

enum class binary_predicate_type {
    ge1_and_le2, /* inside an inclusive range */
    gt1_and_lt2, /* inside an exclusive range */
    lt1_or_gt2, /* outside an inclusive range */
    le1_or_ge2, /* outside an exclusive range */
    ge1, /* greater or equal */
    gt1, /* greater than */
    le1, /* less or equal */
    lt1, /* less than */
    eq1, /* equal to */
    ne1, /* not equal */
};

template <class T>
bool binary_predicate(binary_predicate_type type, T value, T predval1, T predval2) {
    switch (type) {
    case binary_predicate_type::ge1_and_le2: return value >= predval1 && value <= predval2;
    case binary_predicate_type::gt1_and_lt2: return value > predval1 && value < predval2;
    case binary_predicate_type::le1_or_ge2: return value <= predval1 || value >= predval2;
    case binary_predicate_type::lt1_or_gt2: return value < predval1 || value > predval2;
    case binary_predicate_type::le1: return value <= predval1;
    case binary_predicate_type::lt1: return value < predval1;
    case binary_predicate_type::ge1: return value >= predval1;
    case binary_predicate_type::gt1: return value > predval1;
    case binary_predicate_type::eq1: return value == predval1;
    case binary_predicate_type::ne1: return value != predval1;
    }
}

template <class T, class Strategy>
struct copy_if {
    size_t operator()(T const *__restrict /*aligned*/in, T *__restrict /*may-not-aligned*/out, size_t size, T predval1, T predval2, binary_predicate_type predtype) const {
        size_t j = 0;
        for (size_t i = 0; i < size; i++) {
            if (binary_predicate(predtype, in[i], predval1, predval2)) {
                out[j++] = in[i];
            }
        }
        return j;
    }
};

template <>
struct copy_if<float, strategy::AVX> {
    size_t operator()(float const *__restrict in, float *__restrict out, size_t size, float predval1, float predval2, binary_predicate_type predtype) const;
};

template <>
struct copy_if<int, strategy::AVX> {
    size_t operator()(int const *__restrict in, int *__restrict out, size_t size, int predval1, int predval2, binary_predicate_type predtype) const;
};

}
