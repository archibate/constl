#pragma once

#include "core.h"
#include "blocked_range.h"
#include <utility>
#include <type_traits>

namespace pbb {

template <class IsFinal = std::false_type, class Index, class Body>
void parallel_scan(blocked_range<Index> range, Body &body) {
    if (range.size() > range.grain()) {
        blocked_range<Index> right(range, split{});
        Body rbody(body, split{});
        {
            task_group tg;
            tg.run([&rbody, right] {
                parallel_scan<IsFinal>(right, rbody);
            });
            parallel_scan<IsFinal>(range, body);
        }
        if constexpr (!IsFinal::value) {
            task_group tg;
            tg.run([&rbody, right] {
                parallel_scan<std::true_type>(right, rbody);
            });
            parallel_scan<std::true_type>(range, body);
        }
        body.join(rbody);
    } else {
        body(std::as_const(range), IsFinal{});
    }
}

template <class T, class Body, class Join>
struct simple_scan_body {
private:
    T m_result;
    Body m_body;
    Join m_join;

public:
    explicit simple_scan_body(T init, Body body, Join join)
    : m_result(std::move(init))
    , m_body(std::move(body))
    , m_join(std::move(join))
    {}

    template <class Index, class IsFinal>
    void operator()(blocked_range<Index> const &range, IsFinal is_final) {
        m_body(m_result, range, is_final);
    }

    void join(simple_scan_body &that) {
        m_join(m_result, that.m_result);
    }

    explicit simple_scan_body(simple_scan_body &that, split)
    : simple_scan_body(std::as_const(that))
    {}

    [[nodiscard]] T &&result() {
        return std::move(m_result);
    }
};

}
