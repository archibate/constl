#include "test.h"
#include <iostream>
#include <vector>

namespace contest {

static auto &get_cases() {
    static std::vector<std::pair<std::string, std::function<void(ReportHandle *)>>> inst;
    return inst;
}

void _register_test_case(const char *name, std::size_t id, std::string const &type, std::function<void(ReportHandle *)> func) {
    std::string casename = name;
    if (type.size()) {
        casename += '<';
        if (type.find("std::tuple<") == 0 && type.rfind(">") == type.size() - 1) {
            casename.append(type.data() + 11, type.data() + type.size() - 1);
        } else {
            casename += type;
        }
        casename += '>';
    }
    casename += '(';
    casename += std::to_string(id);
    casename += ')';
    get_cases().emplace_back(std::move(casename), std::move(func));
}

static void run_all_cases() {
    std::size_t total = 0, failed = 0, passed = 0;
    std::cerr << "Running " << get_cases().size() << " tests...\n";
    for (auto const &[casename, casefunc]: get_cases()) {
        ReportHandle report;
        report.casename = casename.c_str();
        casefunc(&report);
        if (report.fail_count) {
            ++failed;
        } else {
            ++passed;
        }
        ++total;
    }
    std::cerr << "Ran " << total << " tests, " << passed << " passed, " << failed << " failed\n";
}

int test_main(int argc, char **argv) {
    run_all_cases();
    return 0;
}

ReportHandle::ReportHandle() = default;

void ReportHandle::report_fail(FailureInfo fail) {
    std::cerr << "[FAILED] " << casename << '\n';
    std::cerr << fail.message << '\n';
    ++fail_count;
}

void ReportHandle::report_pass() {
    /* std::cerr << "[  OK  ] " << casename << '\n'; */
    ++pass_count;
}

#if 0
namespace tests {

TEST(SimpleTest) {
    printf("Hello, world!\n");
}

TEST_PARAMS(SimpleParams, {0, 1, 2, 3, 4});
TEST_TYPES(SimpleTypes, float, int, char, double);

TEST_P(SimpleTestWithParams, SimpleParams) {
    int i = getTestParam();
    printf("%d\n", i);
}

TEST_T(SimpleTestWithTypes, SimpleTypes) {
    printf("%s\n", typeid(TestType).name());
}

}
#endif

}
