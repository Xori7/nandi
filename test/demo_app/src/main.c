#include <stdio.h>
#include "nandi.h"

NMutex mutex;
NLogger logger;

void asdf_func(NTestRunner runner) {
    int i = 0;
    int b = i + 5;
    n_test_assert_true(runner, b);
    n_test_assert_true(runner, i);
}

void example_thread(char *text) {
    printf("Thread %llu started\n", n_threading_thread_get_id(n_threading_get_current_thread()));
    for (int i = 0; i <10; ++i) {
        n_threading_mutex_wait(mutex);
        n_logger_log(logger, LOGLEVEL_INFO, "This is info");
        n_logger_log(logger, LOGLEVEL_WARNING, "This is warning");
        n_logger_log(logger, LOGLEVEL_ERROR, "This is error");
        n_logger_log_format(logger, LOGLEVEL_DEBUG, "This is a number: %d, and this is a string: %s", 5, "string");
        n_threading_mutex_release(mutex);
    }
    n_logger_log(logger, LOGLEVEL_DEBUG, "This is debug");
    NTestRunner runner = n_test_runner_create(logger);
    asdf_func(runner);
    n_test_runner_destroy(runner);
}

int main() {
    logger = n_logger_create(LOGGERMODE_CONSOLE | LOGGERMODE_FILE, "logxd.txt");
    NContext context = n_context_create();
    mutex = n_threading_mutex_create();
    NThread thread = n_threading_thread_create((void (*)(void *)) example_thread, "Hello from thread!");
    NThread thread1 = n_threading_thread_create((void (*)(void *)) example_thread, "Hello from cat!");
    n_threading_thread_wait(thread1);
    n_threading_thread_wait(thread);
    n_context_destroy(context);
    n_logger_destroy(logger);
    return 0;
}