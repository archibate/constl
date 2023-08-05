#pragma once

#include <utility>
#include <memory>

namespace constl {

template <class T>
struct NoinitAllocator {
    std::allocator<T> m_base;

    using value_type = T;

    NoinitAllocator() = default;

    template <class ...Args>
    decltype(auto) allocate(Args &&...args) { // 分配与释放功能保持不变，我们只需要劫持构造功能的逻辑
        return m_base.allocate(std::forward<Args>(args)...);
    }

    template <class ...Args>
    decltype(auto) deallocate(Args &&...args) { // 全部给我完美转发过去
        return m_base.deallocate(std::forward<Args>(args)...);
    }

    template <class ...Args>
    void construct(T *p, Args &&...args) {
        if constexpr (!(sizeof...(Args) == 0 && std::is_pod_v<T>)) // 如果是无参构造且类型为POD类型，则不0初始化
            ::new((void *)p) T(std::forward<Args>(args)...);       // 这样的话你仍然可以用 resize(n, 0) 来强制0初始化
    }

    template <class U> // 下面这两个函数是为了伺候 MSVC 编译通过
    constexpr NoinitAllocator(NoinitAllocator<U> const &other) noexcept {}

    constexpr bool operator==(NoinitAllocator<T> const &other) const {
        return this == &other;
    }
};

}
