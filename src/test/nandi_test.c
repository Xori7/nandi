#include <malloc.h>
#include <stdarg.h>
#include "../nandi_internal.h"

extern NTestRunner n_test_runner_create(NLogger logger) {
    NTestRunner runner = malloc(sizeof *runner);
    runner->logger = logger;
    runner->passedTestCount = 0;
    runner->allTestCount = 0;
    return runner;
}

extern void n_test_runner_destroy(NTestRunner testRunner) {
    n_logger_log_format(testRunner->logger, LOGLEVEL_TEST,
                        "Running tests has finished. %d/%d assertions has passed",
                        testRunner->passedTestCount, testRunner->allTestCount);
    free(testRunner);
}

void n_internal_test_assert_equal(NTestRunner testRunner, const char *testName, int32_t testLine, bool condition, const char *format1, const char *format2, ...) {
    if (condition) {
        n_logger_log_format(testRunner->logger, LOGLEVEL_TEST, "%s(line: %d) has passed", testName, testLine);
        testRunner->passedTestCount++;
    }
    else {
        const char *detailsFormat = n_cstring_format("%s%s%s%s", "\n\texpected: ", format1, "\n\tactual: ", format2);
        va_list args;
        va_start(args, format2);
        const char *detailsText = n_internal_cstring_format_args(detailsFormat, args);
        va_end(args);
        n_logger_log_format(testRunner->logger, LOGLEVEL_TEST, "%s(line: %d) has failed%s", testName, testLine, detailsText);
    }
    testRunner->allTestCount++;
}
