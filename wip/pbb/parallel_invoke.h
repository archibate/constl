#pragma once

#include "core.h"
#include "blocked_range.h"
#include <tuple>

namespace pbb {

template <class F1, class ...Fs>
void parallel_invoke(F1 &&f1, Fs &&...fs) {
    task_group tg;
    if constexpr (sizeof...(Fs)) {
        tg.run([&] {
            parallel_invoke(std::forward<Fs>(fs)...);
        });
    }
    std::forward<F1>(f1)();
}

}
