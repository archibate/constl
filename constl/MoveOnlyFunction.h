#pragma once

#include <memory>
#include <utility>

template <class Fn>
class move_only_function {
};

template <class Ret, class ...Args>
class move_only_function<Ret(Args...)> {
    struct Base {
        virtual void operator()(Args &&...) = 0;
    };

    template <class Fn>
    struct Impl : Base {
        Fn m_fn;

        Impl(Fn fn) noexcept : m_fn(std::forward<Fn>(fn)) {}

        void operator()(Args &&...args) override {
            m_fn(std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<Base> m_p;

public:
    template <class Fn, std::enable_if_t<
        !std::is_same_v<std::remove_cv_t<
            std::remove_reference_t<Fn>>, move_only_function> &&
        std::is_invocable_r_v<Ret, Fn &, Args...>, int> = 0>
    move_only_function(Fn &&fn)
    : m_p(std::make_unique<Impl<decltype(fn)>>(std::forward<Fn>(fn)))
    {}

    bool valid() const noexcept {
        return !!m_p;
    }

    void operator()(Args &&...args) {
        m_p->operator()(std::forward<Args>(args)...);
    }
};

template <class Ret, class ...Args>
class move_only_function<Ret(Args...) const> {
    struct Base {
        virtual void operator()(Args &&...) = 0;
    };

    template <class Fn>
    struct Impl : Base {
        Fn m_fn;

        Impl(Fn fn) noexcept : m_fn(std::forward<Fn>(fn)) {}

        void operator()(Args &&...args) const override {
            m_fn(std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<Base> m_p;

public:
    template <class Fn, std::enable_if_t<
        !std::is_same_v<std::remove_cv_t<
            std::remove_reference_t<Fn>>, move_only_function> &&
        std::is_invocable_r_v<Ret, Fn const &, Args...>, int> = 0>
    move_only_function(Fn &&fn)
    : m_p(std::make_unique<Impl<decltype(fn)>>(std::forward<Fn>(fn)))
    {}

    bool valid() const noexcept {
        return !!m_p;
    }

    void operator()(Args &&...args) const {
        m_p->operator()(std::forward<Args>(args)...);
    }
};

template <class Ret, class ...Args>
class move_only_function<Ret(Args...) noexcept> {
    struct Base {
        virtual void operator()(Args &&...) = 0;
    };

    template <class Fn>
    struct Impl : Base {
        Fn m_fn;

        Impl(Fn fn) noexcept : m_fn(std::forward<Fn>(fn)) {}

        void operator()(Args &&...args) noexcept override {
            m_fn(std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<Base> m_p;

public:
    template <class Fn, std::enable_if_t<
        !std::is_same_v<std::remove_cv_t<
            std::remove_reference_t<Fn>>, move_only_function> &&
        std::is_nothrow_invocable_r_v<Ret, Fn &, Args...>, int> = 0>
    move_only_function(Fn &&fn)
    : m_p(std::make_unique<Impl<decltype(fn)>>(std::forward<Fn>(fn)))
    {}

    bool valid() const noexcept {
        return !!m_p;
    }

    void operator()(Args &&...args) noexcept {
        m_p->operator()(std::forward<Args>(args)...);
    }
};

template <class Ret, class ...Args>
class move_only_function<Ret(Args...) const noexcept> {
    struct Base {
        virtual void operator()(Args &&...) = 0;
    };

    template <class Fn>
    struct Impl : Base {
        Fn m_fn;

        Impl(Fn fn) noexcept : m_fn(std::forward<Fn>(fn)) {}

        void operator()(Args &&...args) const noexcept override {
            m_fn(std::forward<Args>(args)...);
        }
    };

    std::unique_ptr<Base> m_p;

public:
    template <class Fn, std::enable_if_t<
        !std::is_same_v<std::remove_cv_t<
            std::remove_reference_t<Fn>>, move_only_function> &&
        std::is_nothrow_invocable_r_v<Ret, Fn const &, Args...>, int> = 0>
    move_only_function(Fn &&fn)
    : m_p(std::make_unique<Impl<decltype(fn)>>(std::forward<Fn>(fn)))
    {}

    bool valid() const noexcept {
        return !!m_p;
    }

    void operator()(Args &&...args) const noexcept {
        m_p->operator()(std::forward<Args>(args)...);
    }
};
