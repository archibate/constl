#include "core.h"
#include <deque>
#include <mutex>
#include <thread>
#include <random>
#include <vector>
#include <optional>
#include <stdexcept>
#include <cstddef>

namespace pbb {

namespace {

struct task_queue {
private:
    mutable std::mutex mtx;
    std::deque<task_functor> tasks;

public:
    void push(task_functor t) {
        std::lock_guard<std::mutex> lck(mtx);
        tasks.push_back(std::move(t));
    }

    [[nodiscard]] std::optional<task_functor> pop() {
        std::lock_guard<std::mutex> lck(mtx);
        if (tasks.empty()) return std::nullopt;
        task_functor t = std::move(tasks.back());
        tasks.pop_back();
        return t;
    }
};

struct task_worker_data {
    task_queue que;
    std::thread thr;
};

typedef size_t task_worker_id;

struct task_state {
    static constexpr task_worker_id null_wid = task_worker_id(-1);
    task_worker_id wid = null_wid;
    std::mt19937 rng;

    void init_state(task_worker_id wid_) {
        wid = wid_;
        rng = std::mt19937(wid);
    }

    task_state &operator=(task_state &&) = delete;
};

struct task_pool {
private:
    static inline thread_local task_state this_task;
    static inline std::vector<task_worker_data> workers;

public:
    [[nodiscard]] static task_worker_id get_this_worker_id() noexcept {
        return this_task.wid;
    }

    static void look_for_task() {
        task_worker_id wid = this_task.wid;
        [[unlikely]] if (this_task.wid == this_task.null_wid)
            throw std::runtime_error("use begin_arena to enter worker 0 first");
        for (size_t time = 0; time < workers.size(); time++) {
            if (auto t = workers[wid].que.pop()) {
                std::move(*t)();
            }
            std::uniform_int_distribution<task_worker_id> dist(0, workers.size() - 1);
            wid = dist(this_task.rng);
        }
    }

    static void begin_arena(task_functor t) {
        if (this_task.wid != this_task.null_wid) {
            std::move(t)();
        } else {
            workers.front().que.push(std::move(t));
        }
    }

    static void spawn_task(task_functor t) {
        [[unlikely]] if (this_task.wid == this_task.null_wid)
            throw std::runtime_error("use begin_arena to enter worker 0 first");
        workers[this_task.wid].que.push(std::move(t));
    }

    static task_pool &ensure_instance() {
        static task_pool instance;
        return instance;
    }

    task_pool() {
        const size_t num_workers = std::thread::hardware_concurrency();
        workers = std::vector<task_worker_data>(num_workers);
        for (size_t i = 0; i < num_workers; i++) {
            std::thread thr([this, i] {
                this_task.init_state(i);
                while (true) {
                    look_for_task();
                }
            });
            workers[i].thr = std::move(thr);
        }
    }

    ~task_pool() noexcept {
        for (size_t i = 0; i < workers.size(); i++) {
            if (workers[i].thr.joinable())
                workers[i].thr.join();
        }
    }

    task_pool &operator=(task_pool &&) = delete;
};

}

void details::begin_arena_impl(task_functor t, std::atomic_size_t *pending) {
    pending->fetch_add(1);
    task_pool::ensure_instance().begin_arena(std::move(t));
    while (pending->load() != 0) {
        std::this_thread::yield();
    }
}

task_group::task_group()
: m_pending(0)
{}

void task_group::_run_task(task_functor t) {
    m_pending.fetch_add(1);
    task_pool::spawn_task(std::move(t));
}

void task_group::wait() {
    while (m_pending.load() != 0) {
        task_pool::look_for_task();
    }
}

task_group::~task_group() noexcept {
    wait();
}

size_t get_this_worker_id() noexcept {
    return task_pool::get_this_worker_id();
}

}
