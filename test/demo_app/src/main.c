#include <stdio.h>
#include "nandi.h"

NandiMutex mutex;

void example_thread(char *text) {
    for (int i = 0; i <10; ++i) {
        nandi_threading_mutex_wait(mutex);
        nandi_threading_thread_sleep(1);
        printf("%s", text);
        nandi_threading_thread_sleep(1);
        printf("%s", text);
        nandi_threading_thread_sleep(1);
        printf("%s", text);
        nandi_threading_mutex_release(mutex);
    }
}

int main() {
    NandiContext context = nandi_context_create();
    mutex = nandi_threading_mutex_create();
    NandiThread thread = nandi_threading_thread_create((void (*)(void *)) example_thread, "Hello from thread!\n");
    NandiThread thread1 = nandi_threading_thread_create((void (*)(void *)) example_thread, "Hello from cat!\n");
    nandi_threading_thread_wait(thread1);
    nandi_threading_thread_wait(thread);
    nandi_context_destroy(context);
    return 0;
}