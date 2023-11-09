#include <malloc.h>
#include "nandi_internal.h"

NLogger nMemoryDebugLogger = NULL;
volatile bool isLogging = false;
NMutex mutex = NULL;

typedef struct {
    void *pointer;
    int32_t generation;
    size_t size;
    const char *func;
    int32_t line;
} Memory;

typedef struct {
    uint32_t memoryId;
    uint32_t generation;
} NSafePtr;

Memory memory[100000] = {0};
int index = 0;

#define deref(type, var, ptr) \
    ;if (memory[ptr.memoryId].generation == ptr.generation) \
        var = *(type*)(memory[ptr.memoryId].pointer); \
    else \
        n_logger_log(nMemoryDebugLogger, LOGLEVEL_ERROR, "Null reference!"); \

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
        NSafePtr ptr = {
                .memoryId = index,
                .generation = 1
        };

        if (!nMemoryDebugLogger) {
            nMemoryDebugLogger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
        }
        int n;
        int c deref(int, c, ptr)
        memory[index] = mem;
        deref(int, b, ptr)

        index++;

        n_logger_log_format(nMemoryDebugLogger, LOGLEVEL_DEBUG, "[MEMORY] Allocated %lld bytes. Func: %s, Line: %d",
                            size, function, line);
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
        Memory mem = {0};
        for (int i = 0; i < index; ++i) {
            if (pointer == memory[i].pointer) {
                mem = memory[i];
                memory[i].pointer = NULL;
                break;
            }
        }

        if (!nMemoryDebugLogger) {
            nMemoryDebugLogger = n_logger_create(LOGGERMODE_CONSOLE, NULL);
        }
        n_logger_log_format(nMemoryDebugLogger, LOGLEVEL_DEBUG,
                            "[MEMORY] Freed %lld bytes at %s(Line: %d)\n\tCreated at: %s(Line: %d)", mem.size, function,
                            line, mem.func, mem.line);
        isLogging = false;
    }
    n_threading_mutex_release(mutex);
    free(pointer);
}

void n_memory_summary() {
    isLogging = true;
    n_logger_log_format(nMemoryDebugLogger, LOGLEVEL_INFO, "[MEMORY] Not deallocated memory regions: ");
    for (int i = 0; i < index; ++i) {
        if (memory[i].pointer == NULL)
            continue;
        Memory mem = memory[i];
        n_logger_log_format(nMemoryDebugLogger, LOGLEVEL_INFO, "[MEMORY] [%d]: Address %p, Size: %lld allocated at %s(Line: %d)", i, mem.pointer, mem.size, mem.func, mem.line);
    }
    isLogging = false;
}