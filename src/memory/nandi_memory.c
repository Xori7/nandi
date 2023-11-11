#include <malloc.h>
#include "../nandi.h"

NMutex mutex = NULL;
bool isLogging = false;

typedef struct {
    void *pointer;
    int32_t generation;
    size_t size;
    const char *func;
    int32_t line;
} Memory;

NList memoryList = {0};

void *n_memory_alloc_debug(size_t size, const char *function, int32_t line) {
    void *ptr = malloc(size);
    if (!mutex) {
        mutex = n_threading_mutex_create();
    }
    n_threading_mutex_wait(mutex);
    if (!isLogging) {
        isLogging = true;
        Memory memory = {
                .pointer = ptr,
                .size = size,
                .func = function,
                .line = line
        };
        if (memoryList.capacity == 0) {
            memoryList = n_list_create(sizeof(Memory), 1024);
        }
        n_list_add(&memoryList, &memory);

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
        for (int i = 0; i < memoryList.count; ++i) {
            if (pointer == n_list_get_inline(memoryList, i, Memory).pointer) {
                n_list_remove_at(&memoryList, i);
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
        for (int i = 0; i < memoryList.count; ++i) {
            Memory mem = n_list_get_inline(memoryList, i, Memory);
            n_logger_log_format(logger, LOGLEVEL_ERROR, "[MEMORY] Not deallocated memory region %u, address %p, allocated %llu bytes at %s(line: %d)", i, mem.pointer,
                                mem.size, mem.func, mem.line);
        }
        isLogging = false;
    }
    n_threading_mutex_release(mutex);
}