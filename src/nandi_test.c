#include <stdarg.h>
#include "nandi.h"

NTestRunner testRunner;

extern void n_test_runner_start(NLogger logger) {
    testRunner = n_memory_alloc(sizeof *testRunner);
    testRunner->logger = logger;
    testRunner->passedTestCount = ATOMIC_INT_LOCK_FREE;
    testRunner->allTestCount = ATOMIC_INT_LOCK_FREE;
}

extern void n_test_runner_finish() {
    n_logger_log_format(testRunner->logger, LOGLEVEL_TEST,
                        "Running tests has finished. %d/%d assertions passed",
                        atomic_load(&testRunner->passedTestCount), atomic_load(&testRunner->allTestCount));
    n_memory_free(testRunner);
}

void i_n_test_assert(const char *testName, int32_t testLine, bool condition, const char *format1, const char *format2, ...) {
    if (condition) {
        n_logger_log_format(testRunner->logger, LOGLEVEL_TEST, "%s(line: %d) has passed", testName, testLine);
        atomic_fetch_add(&testRunner->passedTestCount, 1);
    }
    else {
        char *detailsFormat = n_cstring_format("%s%s%s%s", "\n\texpected: ", format1, "\n\tactual: ", format2);
        va_list args;
        va_start(args, format2);
        char *detailsText = n_internal_cstring_format_args(detailsFormat, args);
        va_end(args);
        n_logger_log_format(testRunner->logger, LOGLEVEL_TEST, "%s(line: %d) has failed%s", testName, testLine, detailsText);
        n_memory_free(detailsFormat);
        n_memory_free(detailsText);
    }
    atomic_fetch_add(&testRunner->allTestCount, 1);
}
