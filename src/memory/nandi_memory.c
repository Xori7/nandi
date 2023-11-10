#include <malloc.h>
#include "../nandi_internal.h"

NMutex mutex = NULL;
bool isLogging = false;

typedef struct {
    void *pointer;
    int32_t generation;
    size_t size;
    const char *func;
    int32_t line;
} Memory;

Memory memory[100000] = {0};
int index = 0;

void *n_memory_alloc_debug(size_t size, const char *function, int32_t line) {
    void *ptr = malloc(size);
    if (!mutex) {
        mutex = n_threading_mutex_create();
    }
    n_threading_mutex_wait(mutex);
    if (!isLogging) {
        isLogging = true;
        Memory mem = {
                .pointer = ptr,
                .size = size,
                .func = function,
                .line = line
        };

        memory[index] = mem;
        index++;
        isLogging = false;
    }
    n_threading_mutex_release(mutex);
    return ptr;
}

void n_memory_free_debug(void *pointer, const char *function, int32_t line) {
    if (!mutex) {
        mutex = n_threading_mutex_create();
    }
    n_threading_mutex_wait(mutex);
    if (!isLogging) {
        isLogging = true;
        for (int i = 0; i < index; ++i) {
            if (pointer == memory[i].pointer) {
                memory[i].pointer = NULL;
                break;
            }
        }
        isLogging = false;
    }
    n_threading_mutex_release(mutex);
    free(pointer);
}

void n_memory_summary(NLogger logger) {
    if (!mutex) {
        mutex = n_threading_mutex_create();
    }
    n_threading_mutex_wait(mutex);
    if (!isLogging) {
        isLogging = true;
        n_logger_log_format(logger, LOGLEVEL_INFO, "[MEMORY] Not deallocated memory regions: ");
        for (int i = 0; i < index; ++i) {
            if (memory[i].pointer == NULL)
                continue;
            Memory mem = memory[i];
            n_logger_log_format(logger, LOGLEVEL_ERROR, "[MEMORY] [%d]: Address %p, Size: %lld allocated at %s(line: %d)", i, mem.pointer,
                                mem.size, mem.func, mem.line);
        }
        isLogging = false;
    }
    n_threading_mutex_release(mutex);
}