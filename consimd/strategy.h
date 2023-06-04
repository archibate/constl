#pragma once

namespace consimd {

namespace strategy {

struct Scalar {};
struct SSE : Scalar {};
struct AVX : SSE {};

}

}
