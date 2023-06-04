#pragma once

#include <utility>
#include <mutex>
#include <shared_mutex>

template <class T, class Mutex = std::mutex>
struct MutexGuard {
    Mutex m_mutex;
    T m_inner;

    struct Locked : std::unique_lock<Mutex> {
        T &m_inner;

        Locked(MutexGuard &parent, auto &&...args)
        noexcept(noexcept(std::unique_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)))
        : std::unique_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)
        , m_inner(parent.m_inner)
        {}

        T &operator*() const noexcept {
            return m_inner;
        }

        T *operator->() const noexcept {
            return std::addressof(m_inner);
        }
    };

    Locked lock(auto &&...args) noexcept(noexcept(Locked(*this, std::forward<decltype(args)>(args)...))) {
        return Locked(*this, std::forward<decltype(args)>(args)...);
    }

    T &get_unsafe() {
        return m_inner;
    }

    T const &get_unsafe() const {
        return m_inner;
    }
};

template <class T, class Mutex = std::shared_mutex>
struct RWMutexGuard {
    mutable Mutex m_mutex;
    T m_inner;

    struct WLocked : std::unique_lock<Mutex> {
        T &m_inner;

        WLocked(RWMutexGuard &parent, auto &&...args)
        noexcept(noexcept(std::unique_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)))
        : std::unique_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)
        , m_inner(parent.m_inner)
        {}

        T &operator*() const noexcept {
            return m_inner;
        }

        T *operator->() const noexcept {
            return std::addressof(m_inner);
        }
    };
    struct RLocked : std::shared_lock<Mutex> {
        T const &m_inner;

        RLocked(RWMutexGuard const &parent, auto &&...args)
        noexcept(noexcept(std::shared_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)))
        : std::unique_lock<Mutex>(parent.m_mutex, std::forward<decltype(args)>(args)...)
        , m_inner(parent.m_inner)
        {}

        T const &operator*() const noexcept {
            return m_inner;
        }

        T const *operator->() const noexcept {
            return std::addressof(m_inner);
        }
    };

    WLocked write(auto &&...args) noexcept(noexcept(WLocked(*this, std::forward<decltype(args)>(args)...))) {
        return WLocked(*this, std::forward<decltype(args)>(args)...);
    }

    RLocked read(auto &&...args) const noexcept(noexcept(RLocked(*this, std::forward<decltype(args)>(args)...))) {
        return RLocked(*this, std::forward<decltype(args)>(args)...);
    }

    T &get_unsafe() {
        return m_inner;
    }

    T const &get_unsafe() const {
        return m_inner;
    }
};
