#pragma once

#include "core.h"
#include "blocked_range.h"
#include <utility>

namespace pbb {

template <class Index, class Body>
void parallel_reduce(blocked_range<Index> range, Body &body) {
    if (range.size() > range.grain()) {
        blocked_range<Index> right(range, split{});
        Body rbody(body, split{});
        {
            task_group tg;
            tg.run([&rbody, right] {
                parallel_reduce(right, rbody);
            });
            parallel_reduce(range, body);
        }
        body.join(rbody);
    } else {
        body(std::as_const(range));
    }
}

template <class T, class Body, class Join>
struct simple_reduce_body {
private:
    T m_result;
    Body m_body;
    Join m_join;
    T m_init;

public:
    explicit simple_reduce_body(T init, Body body, Join join)
    : m_result(std::move(init))
    , m_body(std::move(body))
    , m_join(std::move(join))
    {
        m_init = std::as_const(m_result);
    }

    template <class Index>
    void operator()(blocked_range<Index> const &range) {
        m_body(m_result, range);
    }

    void join(simple_reduce_body &that) {
        m_join(m_result, that.m_result);
    }

    explicit simple_reduce_body(simple_reduce_body &that, split)
    : simple_reduce_body(that.m_init, that.m_body, that.m_join)
    {}

    [[nodiscard]] T &&result() {
        return std::move(m_result);
    }
};

template <class T, class Body, class Join>
simple_reduce_body(T init, Body body, Join join) -> simple_reduce_body<T, Body, Join>;

template <class Index, class T, class Body, class Join>
T parallel_reduce(blocked_range<Index> range, T init, Body body, Join join) {
    simple_reduce_body<T, Body, Join> reducer(std::move(init), std::move(body), std::move(join));
    parallel_reduce(range, reducer);
    return reducer.result();
}

}
