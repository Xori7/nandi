#include <malloc.h>
#include "../nandi_internal.h"

extern NTestRunner n_test_runner_create(NLogger logger) {
    NTestRunner runner = malloc(sizeof *runner);
    runner->logger = logger;
    runner->passedTestCount = 0;
    runner->allTestCount = 0;
    return runner;
}

extern void n_test_runner_destroy(NTestRunner testRunner) {
    n_logger_log_format(testRunner->logger, LOGLEVEL_INFO,
                        "Running tests has finished. %d/%d assertions has passed",
                        testRunner->passedTestCount, testRunner->allTestCount);
    free(testRunner);
}

void n_internal_test_assert_true(NTestRunner testRunner, bool value, const char *testName) {
    if (value) {
        n_logger_log_format(testRunner->logger, LOGLEVEL_INFO, "%s has passed", testName);
        testRunner->passedTestCount++;
    }
    else {
        n_logger_log_format(testRunner->logger, LOGLEVEL_INFO, "%s has failed\n\texpected: true\n\tactual: false", testName);
    }
    testRunner->allTestCount++;
}
