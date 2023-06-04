#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <atomic>
#include <deque>

namespace pbb {

struct task_functor;

namespace details {

void begin_arena_impl(task_functor t, std::atomic_size_t *);

struct task_iface {
private:
    std::atomic_size_t *pending;

protected:
    explicit task_iface(std::atomic_size_t *pending_) noexcept
    : pending(pending_) {}

public:
    ~task_iface() noexcept {
        pending->fetch_sub(1);
    }

    task_iface &operator=(task_iface &&) = delete;
};

template <class F>
struct task_impl : public task_iface {
private:
    F fn;

public:
    task_impl(std::atomic_size_t *pending, F fn_) noexcept
    : task_iface(pending), fn(std::move(fn_))
    {}

    ~task_impl() noexcept {
        fn();
    }
};

}

struct [[nodiscard]] task_functor {
private:
    std::shared_ptr<details::task_iface> m_inner;

public:
    template <class F, std::enable_if_t<std::is_invocable_v<F>, int> = 0>
    explicit task_functor(F fn, std::atomic_size_t *pending)
    : m_inner(std::make_shared<details::task_impl<F>>(pending, std::move(fn))) {
    }

    void operator()() {
        m_inner = nullptr;
    }

    task_functor(task_functor &&) = default;
    task_functor &operator=(task_functor &&) = default;
    task_functor(task_functor const &) = delete;
    task_functor &operator=(task_functor const &) = delete;
};

struct task_group {
private:
    std::atomic_size_t m_pending;

    void _run_task(task_functor t);

public:
    task_group();
    void wait();
    template <class F, std::enable_if_t<std::is_invocable_v<F>, int> = 0>
    void run(F &&fn) {
        _run_task(task_functor(std::forward<F>(fn), &m_pending));
    }

    task_group(task_group &&) = delete;
    task_group &operator=(task_group &&) = delete;
    task_group(task_group const &) = delete;
    task_group &operator=(task_group const &) = delete;

    ~task_group() noexcept;
};

template <class F>
void begin_arena(F &&fn) {
    std::atomic_size_t pending(0);
    details::begin_arena_impl(task_functor(std::forward<F>(fn), &pending), &pending);
}

size_t get_this_worker_id() noexcept;

}
