#include "nandi.h"

NTestRunner testRunner;

void test_example(int a, int b) {
    n_test_assert_true(testRunner, !b);
    n_test_assert_false(testRunner, !b);
    n_test_assert_int32_equal(testRunner, a, b);
    n_test_assert_int32_greater(testRunner, a, 1);
    n_test_assert_int32_lower(testRunner, a, -1);
}

int main() {
    NLogger logger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
    testRunner = n_test_runner_create(logger);
    test_example(2, 0);
    test_example(-2, 4);
    test_example(3, 3);
    n_test_runner_destroy(testRunner);
    n_logger_destroy(logger);
    return 0;
}