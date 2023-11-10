#include "nandi_test.h"


int main() {
    logger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
    testRunner = n_test_runner_create(logger);

    test_n_context();

    n_test_runner_destroy(testRunner);
    n_logger_destroy(logger);
#ifdef MEMORY_DEBUG
    n_memory_summary(logger);
#endif
    return 0;
}