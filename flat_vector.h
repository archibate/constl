#pragma once

#include <memory>
#include <utility>
#include <concepts>

template
< class T
, class Alloc = std::allocator<T>
>
class flat_vector {
};
