#include "nandi.h"

NTestRunner testRunner;

void test_example(int a, int b) {
    n_test_assert_true(testRunner, a == b);
}

int main() {
    NLogger logger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
    testRunner = n_test_runner_create(logger);
    test_example(2, 4);
    test_example(-2, 4);
    test_example(3, 3);
    n_test_runner_destroy(testRunner);
    n_logger_destroy(logger);
    return 0;
}