#pragma once

namespace psimd {

namespace strategy {

struct Scalar {};
struct SSE : Scalar {};
struct AVX : SSE {};

}

}
