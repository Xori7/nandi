#include <stdio.h>
#include "nandi.h"

NandiMutex mutex;

void example_thread(char *text) {
    printf("Thread %llu started\n", nandi_threading_thread_get_id(nandi_threading_get_current_thread()));
    for (int i = 0; i <10; ++i) {
        nandi_threading_mutex_wait(mutex);
        nandi_logger_log(LOGLEVEL_INFO, "This is info");
        nandi_logger_log(LOGLEVEL_WARNING, "This is warning");
        nandi_logger_log(LOGLEVEL_ERROR, "This is error");
        nandi_threading_mutex_release(mutex);
    }
    nandi_logger_log(LOGLEVEL_DEBUG, "This is debug");
}

int main() {
    NandiContext context = nandi_context_create();
    nandi_logger_initialize(LOGGERMODE_CONSOLE | LOGGERMODE_FILE);
    mutex = nandi_threading_mutex_create();
    NandiThread thread = nandi_threading_thread_create((void (*)(void *)) example_thread, "Hello from thread!");
    NandiThread thread1 = nandi_threading_thread_create((void (*)(void *)) example_thread, "Hello from cat!");
    nandi_threading_thread_wait(thread1);
    nandi_threading_thread_wait(thread);
    nandi_context_destroy(context);
    return 0;
}