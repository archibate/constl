#include "test.h"

namespace contest {

static auto &get_cases() {
    static std::vector<std::pair<std::string, std::function<void(ReportHandle *)>>> inst;
    return inst;
}

void _register_test_case(const char *name, std::size_t id, std::function<void(ReportHandle *)> func) {
    std::string casename = name;
    casename += '/';
    casename += std::to_string(id);
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

namespace tests {

TEST_CASE(SimpleTest) {
    printf("Hello, world!\n");
}

TEST_PARAMS(SimpleParams, int, 0, 1, 2, 3, 4);
TEST_TYPES(SimpleTypes, float, int, char, double);

TEST_CASE_P(SimpleTestWithParams, SimpleParams) {
    int i = get_param();
    printf("%d\n", i);
}

TEST_CASE_T(SimpleTestWithTypes, SimpleTypes) {
    printf("%s\n", typeid(TestType).name());
}

}

}
