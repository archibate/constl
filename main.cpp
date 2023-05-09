#if 1
#include "contest/test.h"

int main(int argc, char **argv) {
    return contest::test_main(argc, argv);
}
#else
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
