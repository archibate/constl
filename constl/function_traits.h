#include "extra_traits.h"
#include "../contest/test.h"

namespace constl {

TEST_BEGIN()

TEST(FunctionTraits) {
    {
        using F = std::function<std::pair<int &, char *>(float, double &, char const *const &)>;
        static_assert(std::is_same_v<function_traits<F>::argument_types, std::tuple<float, double &, char const *const &>>);
        static_assert(std::is_same_v<function_traits<F>::result_type, std::pair<int &, char *>>);
    }
    {
        int x = 42;
        auto f = [&] (char d[3], int *&&s) -> decltype(auto) {
            return s[x >> d[0]];
        };
        using F = decltype(f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        int x = 42;
        auto f = [&] (char d[3], int *&&s) mutable -> decltype(auto) {
            return s[x >> d[0]];
        };
        using F = decltype(f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            static auto f(char d[3], int *&&s) -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            static auto f(char d[3], int *&&s) noexcept -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto f(char d[3], int *&&s) const -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto f(char d[3], int *&&s) -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto f(char d[3], int *&&s) const & noexcept -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto f(char d[3], int *&&s) noexcept -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = decltype(&inplace::f);
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto operator()(char d[3], int *&&s) const -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = inplace;
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto operator()(char d[3], int *&&s) && -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = inplace;
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto operator()(char d[3], int *&&s) const noexcept -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = inplace;
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
    {
        const int x = 42;
        struct inplace {
            auto operator()(char d[3], int *&&s) & noexcept -> decltype(auto) {
                return s[x >> d[0]];
            };
        };
        using F = inplace;
        static_assert(std::is_same_v<typename function_traits<F>::argument_types, std::tuple<char *, int *&&>>);
        static_assert(std::is_same_v<typename function_traits<F>::result_type, int &>);
    }
}

TEST_END()

}
