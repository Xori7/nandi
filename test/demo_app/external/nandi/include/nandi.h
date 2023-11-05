#ifndef NANDI
#define NANDI

#include <stdbool.h>
#include <stdint.h>

// Threading
typedef void *NandiThread;
typedef void *NandiMutex;

extern NandiThread nandi_threading_thread_create(void (*func)(void*), void* args); // Creates new thread and executes func with args
extern void nandi_threading_thread_wait(NandiThread thread); //Waits until thread finishes its execution
extern void nandi_threading_thread_terminate(NandiThread thread, int exitCode); // Terminates thread with exitCode
extern uint64_t nandi_threading_thread_get_id(NandiThread thread);
extern void nandi_threading_thread_sleep(uint64_t milliseconds);
extern NandiThread nandi_threading_get_current_thread();

extern NandiMutex nandi_threading_mutex_create(); // Creates a mutex
extern bool nandi_threading_mutex_wait(NandiMutex mutex); // Waits until mutex is unlocked and locks it for current thread
extern bool nandi_threading_mutex_release(NandiMutex mutex); // Releases mutex lock state

// Logger
typedef enum {
    LOGGERMODE_CONSOLE = 0b01,
    LOGGERMODE_FILE = 0b10
} NandiLoggerMode;

typedef enum {
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARNING,
    LOGLEVEL_ERROR
} NandiLogLevel;

extern void nandi_logger_initialize(NandiLoggerMode mode); // Initializes logger with specific mode. IMPORTANT: Should be called only once before any nandi_logger_log call
extern void nandi_logger_log(NandiLogLevel level, char *message); // Logs message and marks it with specific log level

// Context
#ifndef NANDI_INTERNAL
typedef void *NandiContext;
#endif

extern NandiContext nandi_context_create();
extern void nandi_context_destroy(NandiContext context);

#endif
