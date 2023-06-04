#pragma once

#include <vector>
#include <random>
#include <utility>
#include <numeric>
#include <algorithm>
#include <functional>
#include <concepts>
#include <type_traits>
#include <thread>
#include <latch>
#include "ConcurrentQueue.h"
#include "../constl/move_only_function.h"
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <sched.h>
#endif

namespace conpool {

struct ThreadPool {
    struct ThreadData {
        std::jthread m_thread;
        ConcurrentQueue<move_only_function<void()>> m_task_queue;
    };

    std::vector<ThreadData> m_threads;

    static bool set_thread_affinity(std::thread::native_handle_type handle, int cpu) {
#if defined(_WIN32)
        DWORD_PTR mask = 1 << cpu;
        DWORD_PTR result = SetThreadAffinityMask(handle, mask);
        if (result == 0) [[unlikely]] {
            return false;
        }
#elif defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        int result = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
        if (result != 0) [[unlikely]] {
            return false;
        }
#endif
        return true;
    }

    ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {
        for (std::size_t i = 0; i < m_threads.size(); ++i) {
            set_thread_affinity(m_threads[i].m_thread.native_handle(), i);
        }
    }

    explicit ThreadPool(std::size_t nthreads) : m_threads(std::max(nthreads, std::size_t(1))) {
        for (auto &thread_data: m_threads) {
            thread_data.m_thread = std::jthread(_thread_entry, this, &thread_data);
        }
    }

    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    ThreadPool(ThreadPool const &) = delete;
    ThreadPool &operator=(ThreadPool const &) = delete;
    ~ThreadPool() noexcept {
        request_stop();
    }

    void request_stop() noexcept {
        for (auto &thread_data: m_threads) {
            if (thread_data.m_thread.joinable())
                thread_data.m_thread.request_stop();
        }
    }

    static inline thread_local struct {
        ThreadPool *this_pool;
        ThreadData *this_thread;
        std::vector<std::size_t> workers;
        std::mt19937 rand_gen;
    } m_tls{nullptr, nullptr};

    static void _look_for_task(std::invocable auto is_stop, ThreadPool *this_pool, ThreadData *this_thread) {
        {
            auto &q = this_thread->m_task_queue;
            while (true) {
                if (is_stop()) return;
                if (auto task = q.try_pop()) {
                    std::move(*task)();
                } else {
                    break;
                }
            }
        }
        auto &workers = m_tls.workers;
        std::shuffle(workers.begin(), workers.end(), m_tls.rand_gen);
        for (auto &worker_id: workers) {
            auto &thread_data = this_pool->m_threads[worker_id];
            if (&thread_data == this_thread) continue;
            if (is_stop()) return;
            auto &q = thread_data.m_task_queue;
            if (auto task = q.try_pop()) {
                std::move(*task)();
            }
        }
    }

    static void _thread_entry(std::stop_token stoken, ThreadPool *this_pool, ThreadData *this_thread) {
        m_tls.this_pool = this_pool;
        m_tls.this_thread = this_thread;
        m_tls.workers.resize(this_pool->m_threads.size());
        m_tls.rand_gen.seed(std::hash<std::thread::id>()(std::this_thread::get_id()));
        std::iota(m_tls.workers.begin(), m_tls.workers.end(), std::size_t(0));
        while (true) {
            _look_for_task([&] {
                if (stoken.stop_requested()) [[unlikely]] return true;
                return false;
            }, this_pool, this_thread);
            if (stoken.stop_requested()) [[unlikely]] return;
            std::this_thread::yield();
        }
    }

    static void join(std::invocable auto &&...fns) requires (sizeof...(fns) != 0) {
        auto this_pool = m_tls.this_pool;
        if (!this_pool) [[unlikely]] {
            return default_pool().arena([&fns...] {
                return join(std::forward<decltype(fns)>(fns)...);
            });
        }
        auto this_thread = m_tls.this_thread;
        std::latch m_latch{sizeof...(fns)};
        {
            auto &q = this_thread->m_task_queue;
            (q.push([&m_latch, &fns] () mutable {
                std::forward<decltype(fns)>(fns)();
                m_latch.count_down();
            }), ...);
        }
        bool ready = false;
        do {
            auto stoken = this_thread->m_thread.get_stop_token();
            _look_for_task([&] {
                if (stoken.stop_requested()) [[unlikely]] return true;
                ready = m_latch.try_wait();
                return ready;
            }, this_pool, this_thread);
            if (stoken.stop_requested()) [[unlikely]] return;
            if (!ready) ready = m_latch.try_wait();
        } while (!ready);
    }

    static bool stop_requested() noexcept {
        return m_tls.this_thread->m_thread.get_stop_token().stop_requested();
    }

    static ThreadPool &this_pool() noexcept {
        if (!m_tls.this_pool) [[unlikely]] std::terminate();
        return *m_tls.this_pool;
    }

    static ThreadPool &default_pool() noexcept {
        static ThreadPool pool;
        return pool;
    }

    void arena(std::invocable auto &&fn) {
        std::latch m_latch{1};
        m_threads.front().m_task_queue.push([&m_latch, &fn] {
            std::forward<decltype(fn)>(fn)();
            m_latch.count_down();
        });
        m_latch.wait();
    }
};

}
