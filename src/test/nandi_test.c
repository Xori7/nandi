#include <malloc.h>
#include <stdio.h>
#include <string.h>
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
    uint32_t length = 128 + strlen(testName);
    char *message = malloc(sizeof *message * length);
    if (value) {
        snprintf(message, length, "%s has passed", testName);
        testRunner->passedTestCount++;
    }
    else {
        snprintf(message, length, "%s has failed\n\texpected: true\n\tactual: false", testName);
    }
    testRunner->allTestCount++;
    n_logger_log(testRunner->logger, LOGLEVEL_INFO, message);
}
