#ifndef NANDI
#define NANDI

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>

// CString
extern char *n_cstring_format(const char *format, ...);

// Threading
typedef void *NThread;
typedef void *NMutex;

extern NThread n_threading_thread_create(void (*func)(void*), void* args); // Creates new thread and executes func with args
extern void n_threading_thread_wait(NThread thread); //Waits until thread finishes its execution
extern void n_threading_thread_terminate(NThread thread, int exitCode); // Terminates thread with exitCode
extern uint64_t n_threading_thread_get_id(NThread thread);
extern void n_threading_thread_sleep(uint64_t milliseconds);
extern NThread n_threading_get_current_thread();

extern NMutex n_threading_mutex_create(); // Creates a mutex
extern bool n_threading_mutex_wait(NMutex mutex); // Waits until mutex is unlocked and locks it for current thread
extern bool n_threading_mutex_release(NMutex mutex); // Releases mutex lock state

// Logger
typedef enum {
    LOGGERMODE_CONSOLE = 0b01,
    LOGGERMODE_FILE = 0b10
} NLoggerMode;

typedef enum {
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARNING,
    LOGLEVEL_ERROR,
    LOGLEVEL_TEST
} NLogLevel;

#ifndef NANDI_INTERNAL
typedef void *NLogger;
#else
typedef struct {
    char *filePath;
    volatile NLoggerMode mode;
    NMutex logMutex;
} *NLogger;
#endif

extern NLogger n_logger_create(NLoggerMode mode, char *filePath); // Initializes logger with specific mode. IMPORTANT: Should be called only once before any n_logger_log call
extern void n_logger_destroy(NLogger logger); //Destroys the logger
extern void n_logger_log(NLogger logger, NLogLevel level, char *message); // Logs message and marks it with specific log level
extern void n_logger_log_format(NLogger logger, NLogLevel level, const char *format, ...); // Logs message, formats it and marks it with specific log level

// Memory
void *n_memory_alloc_debug(size_t size, const char *function, int32_t line);
void n_memory_free_debug(void *pointer, const char *function, int32_t line);
void n_memory_summary(NLogger logger);

#define MEMORY_DEBUG
#ifdef MEMORY_DEBUG
    #define n_memory_alloc(size) n_memory_alloc_debug(size, __func__, __LINE__)
    #define n_memory_free(pointer) n_memory_free_debug(pointer, __func__, __LINE__); pointer = NULL
#else
    #define n_memory_alloc(size) malloc(size)
    #define n_memory_free(pointer) free(pointer); pointer = NULL
#endif

// Memory -> List
typedef struct {
    void *elements;
    int64_t count;
    int64_t capacity;
    size_t typeSize;
} NList;

#define n_list_create(Type, capacity) { n_memory_alloc(sizeof(Type) * capacity), 0, capacity, sizeof(Type) }
#define n_list_destroy(list) n_memory_free(list.elements)
#define n_list_add(Type, list, value) ((Type*)list.elements)[sizeof(Type) * list.count] = value

void foo() {
    NList list = n_list_create(int, 10);
    n_list_add(int, list, 7);
    n_list_destroy(list);
}

// Test
#ifndef NANDI_INTERNAL
typedef void *NTestRunner;
#else
typedef struct {
    NLogger logger;
    volatile uint32_t passedTestCount;
    volatile uint32_t allTestCount;
} *NTestRunner;
#endif

extern NTestRunner n_test_runner_create(NLogger logger);
extern void n_test_runner_destroy(NTestRunner testRunner);

void n_internal_test_assert_equal(NTestRunner testRunner, const char *testName, int32_t testLine, bool condition, const char *expectedFormat, const char *actualFormat, ...);
#define n_test_assert_true(runner, value) n_internal_test_assert_equal(runner, __func__, __LINE__, value, "%s", "%s", "true", "false")
#define n_test_assert_false(runner, value) n_internal_test_assert_equal(runner, __func__, __LINE__, !value, "%s", "%s", "false", "true")
#define n_test_assert_int32_equal(runner, expected, actual) n_internal_test_assert_equal(runner, __func__, __LINE__, actual == expected, "%d", "%d", expected, actual)
#define n_test_assert_int32_greater(runner, a, b) n_internal_test_assert_equal(runner, __func__, __LINE__, a > b, "> %d", "%d", b, a)
#define n_test_assert_int32_lower(runner, a, b) n_internal_test_assert_equal(runner, __func__, __LINE__, a < b, "< %d", "%d", b, a)

// Context
#ifndef NANDI_INTERNAL
typedef void *NContext;
#endif

extern NContext n_context_create(void);
extern void n_context_destroy(NContext context);

#endif
