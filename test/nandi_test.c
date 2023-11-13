#include <stdio.h>
#include "nandi_test.h"

int main() {
    logger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
    n_test_runner_start(logger);

    test_n_list();
    test_n_window();

    n_test_runner_finish();
    n_logger_destroy(logger);
#ifdef MEMORY_DEBUG
    n_memory_summary(logger);
#endif
    return 0;
}