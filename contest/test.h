#pragma once

#include <tuple>
#include <utility>
#include <limits>
#include <type_traits>
#include <string>
#include <functional>
#include "../conutils/cppdemangle.h"

namespace contest {

int test_main(int argc, char **argv);

struct FailureInfo {
    std::string message;
};

struct ReportHandle {
    const char *casename = nullptr;
    std::size_t fail_count = 0, pass_count = 0;

    ReportHandle();
    ReportHandle(ReportHandle &&) = delete;

    void report_fail(FailureInfo fail);
    void report_pass();
};

#define NAMED(x) ::contest::makeNamed(#x, x)
#define EXPECT_NEAR(x, y) expects(::contest::Condition::expect_near(NAMED(x), NAMED(y)))

template <class T>
struct Named {
    const char *name;
    T value;

    std::string as_message() const {
        std::string msg = name;
        msg += " = ";
        msg += std::to_string(value);
        return msg;
    }
};

template <class T>
Named<T> makeNamed(const char *name, T t) {
    return {name, std::move(t)};
}

struct Condition {
    bool value;
    std::string message;

    template <class ...Ts>
    static Condition expect(bool value, Named<Ts> const &...ts) {
        Condition cond;
        ((cond.message += ts.as_message(), cond.message += '\n'), ...);
        cond.value = value;
        return cond;
    }

    template <class T>
    static Condition expect_true(Named<T> const &t) {
        return expect((bool)t.value, t);
    }

    template <class T>
    static Condition expect_false(Named<T> const &t) {
        return expect(!(bool)t.value, t);
    }

    template <class T1, class T2>
    static Condition expect_eq(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value == t2.value, t1, t2);
    }

    template <class T1, class T2>
    static Condition expect_ne(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value != t2.value, t1, t2);
    }

    template <class T1, class T2>
    static Condition expect_lt(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value < t2.value, t1, t2);
    }

    template <class T1, class T2>
    static Condition expect_gt(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value > t2.value, t1, t2);
    }

    template <class T1, class T2>
    static Condition expect_le(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value <= t2.value, t1, t2);
    }

    template <class T1, class T2>
    static Condition expect_ge(Named<T1> const &t1, Named<T2> const &t2) {
        return expect(t1.value >= t2.value, t1, t2);
    }

    template <class T1, class T2, class T = std::common_type_t<T1, T2>>
    static Condition expect_near(Named<T1> const &t1, Named<T2> const &t2) {
        if constexpr (std::is_floating_point_v<T>) {
            const float rel = std::numeric_limits<float>::epsilon() * 15;
            const float abs = std::numeric_limits<float>::denorm_min() * 15;
            return expect(std::abs(t1.value - t2.value) / std::max(std::abs(t1.value + t2.value), abs * 2) <= rel * 2, t1, t2);
        } else {
            return expect(t1.value == t2.value, t1, t2);
        }
    }
};

struct TestBase {
private:
    ReportHandle *m_report;

    template <class Derived>
    friend struct TestCase;

public:
    void expects(Condition cond) {
        if (!cond.value) {
            m_report->report_fail(FailureInfo{std::move(cond.message)});
        } else {
            m_report->report_pass();
        }
    }
};

struct NoTestTypes {
    using template_types = std::tuple<void>;
};

struct NoTestParams {
    using param_type = int;
    static inline const param_type params[] = {0};
};

void _register_test_case(const char *name, std::size_t id, std::string const &type, std::function<void(ReportHandle *)> func);

template <class Derived>
class TestCase : public TestBase {
private:
    template <class T>
    static void _register_over_params() {
        auto type_demangled = conutils::cppdemangle(typeid(T));
        for (std::size_t i = 0; i < std::size(Derived::params); i++) {
            _register_test_case(
                Derived::test_case_name, i, type_demangled,
                [paramPtr = std::addressof(std::begin(Derived::params)[i])] (ReportHandle *rh) {
                    Derived tc;
                    tc.m_param = reinterpret_cast<void const *>(paramPtr);
                    tc.m_report = rh;
                    tc.template do_run<T>();
                });
        }
        
    }

    template <std::size_t ...Is>
    static void _register_impl(std::index_sequence<Is...>) {
        (_register_over_params<std::tuple_element_t<Is, typename Derived::template_types>>(), ...);
    }

    void const *m_param;

protected:
    auto const &getTestParam() const {
        return *reinterpret_cast<typename Derived::param_type const *>(m_param);
    }

public:
    struct register_helper {
        register_helper() {
            if constexpr (std::tuple_size_v<typename Derived::template_types> != 0) {
                _register_impl(std::make_index_sequence<std::tuple_size_v<typename Derived::template_types>>{});
            } else {
                _register_over_params<void>();
            }
        }
    };
};

#define TEST_PARAMS(_name, ...) \
struct TestParams_##_name { \
    static inline const auto params = __VA_ARGS__; \
    using param_type = std::decay_t<decltype(*std::begin(params))>; \
};
#define TEST_PARAMS_TYPED(_type, _name, ...) \
struct TestParams_##_name { \
    using param_type = _type; \
    static inline const param_type params = __VA_ARGS__; \
};
#define TEST_TYPES(_name, ...) \
struct TestTypes_##_name { \
    using template_types = std::tuple<__VA_ARGS__>; \
};

#define TEST_PT(_name, _params, _types) \
struct Test_##_name : TestParams_##_params, TestTypes_##_types, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST_T(_name, _types) \
struct Test_##_name : ::contest::NoTestParams, TestTypes_##_types, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST_P(_name, _params) \
struct Test_##_name : TestParams_##_params, ::contest::NoTestTypes, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST(_name) \
struct Test_##_name : ::contest::NoTestParams, ::contest::NoTestTypes, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

}
