#pragma once

#include "core.h"
#include "blocked_range.h"
#include <utility>

namespace pbb {

template <class Index, class Body>
void parallel_for(blocked_range<Index> range, Body &&body) {
    if (range.size() > range.grain()) {
        blocked_range<Index> right(range, split{});
        task_group tg;
        tg.run([&body, right] {
            parallel_for(right, body);
        });
        parallel_for(range, body);
    } else {
        body(std::as_const(range));
    }
}

}
