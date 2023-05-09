#pragma once

#include <tuple>
#include <utility>
#include <string>
#include <iostream>
#include <functional>

namespace contest {

int test_main(int argc, char **argv);

struct FailureInfo {
    std::string message;
};

struct ReportHandle {
    const char *casename = nullptr;
    std::size_t fail_count = 0, pass_count = 0;

    void report_fail(FailureInfo fail) {
        std::cerr << "[FAILED] " << casename << '\n';
        std::cerr << fail.message << '\n';
        ++fail_count;
    }

    void report_pass() {
        std::cerr << "[  OK  ] " << casename << '\n';
        ++pass_count;
    }
};

struct NoTestTypes {
    using template_types = std::tuple<void>;
};

struct NoTestParams {
    using param_type = int;
    static inline const param_type params[] = {0};
};

void _register_test_case(const char *name, std::size_t id, std::function<void(ReportHandle *)> func);

template <class Derived>
class TestCase {
private:
    template <class T>
    static void _register_over_params() {
        for (std::size_t i = 0; i < std::size(Derived::params); i++) {
            _register_test_case(
                Derived::test_case_name, i,
                [paramPtr = std::addressof(Derived::params[i])] (ReportHandle *rh) {
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
    ReportHandle *m_report;

protected:
    auto const &get_param() const {
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

#define TEST_PARAMS(_name, _partype, ...) \
struct TestParams_##_name { \
    using param_type = _partype; \
    static inline const param_type params[] = { \
        __VA_ARGS__ \
    }; \
};
#define TEST_PARAMS_EX(_name, _partype, _params) \
struct TestParams_##_name { \
    using param_type = _partype; \
    static inline const param_type params[] = _params; \
};
#define TEST_TYPES(_name, ...) \
struct TestTypes_##_name { \
    using template_types = std::tuple<__VA_ARGS__>; \
};

#define TEST_CASE_PT(_name, _params, _types) \
struct Test_##_name : TestTypes_##_params, TestParams_##_types, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST_CASE_T(_name, _types) \
struct Test_##_name : ::contest::NoTestParams, TestTypes_##_types, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST_CASE_P(_name, _params) \
struct Test_##_name : TestParams_##_params, ::contest::NoTestTypes, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

#define TEST_CASE(_name) \
struct Test_##_name : ::contest::NoTestParams, ::contest::NoTestTypes, ::contest::TestCase<Test_##_name> { \
    static inline const char test_case_name[] = #_name; \
    static inline register_helper _register_helper; \
    template <class> \
    inline void do_run(); \
}; \
template <class TestType> \
void Test_##_name::do_run()

}
