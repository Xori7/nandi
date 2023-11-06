#include <malloc.h>
#include <stdio.h>
#include "../nandi_internal.h"

extern NTestRunner n_test_runner_create(NLogger logger) {
    NTestRunner runner = malloc(sizeof *runner);
    runner->logger = logger;
    return runner;
}

extern void n_test_runner_destroy(NTestRunner testRunner) {
    free(testRunner);
}


void n_internal_test_assert_true(NTestRunner testRunner, bool value, const char *testName) {
    n_logger_log(testRunner->logger, LOGLEVEL_INFO, testName);
}
