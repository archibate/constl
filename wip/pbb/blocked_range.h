#pragma once

#include <utility>
#include <algorithm>
#include <type_traits>
#include <cstddef>

namespace pbb {

struct split {};

template <class Index = size_t>
struct blocked_range {
private:
    Index m_begin;
    Index m_end;
    size_t m_grain;

public:
    blocked_range()
    : m_begin(), m_end(), m_grain(0)
    {}

    explicit blocked_range(Index begin, Index end, size_t grain = 0)
    : m_begin(begin), m_end(end), m_grain(grain)
    {}

    Index begin() const {
        return m_begin;
    }

    Index end() const {
        return m_end;
    }

    size_t size() const {
        return m_end - m_begin;
    }

    size_t grain() const {
        return m_grain;
    }

    explicit blocked_range(blocked_range &that, split) {
        Index middle = that.m_begin + (that.size() >> 1);
        m_begin = middle;
        m_end = that.m_end;
        m_grain = that.m_grain;
        that.m_end = middle;
    }
};

template <class Index>
blocked_range(Index begin, Index end, size_t grain = 0) -> blocked_range<Index>;

template <class Index>
blocked_range(blocked_range<Index> &that, split) -> blocked_range<Index>;

}
