#ifndef NANDI
#define NANDI

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <stdatomic.h>

// CString
char *n_internal_cstring_format_args(const char *format, va_list args);
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
static char *logLevelNames[] = {
        "DEBUG",
        "INFO ",
        "WARN ",
        "ERROR",
        "TEST"
};
#define ANSI_COLOR_RED     "\x1b[91m"
#define ANSI_COLOR_GREEN   "\x1b[92m"
#define ANSI_COLOR_YELLOW  "\x1b[93m"
#define ANSI_COLOR_BLUE    "\x1b[94m"
#define ANSI_COLOR_MAGENTA "\x1b[95m"
#define ANSI_COLOR_CYAN    "\x1b[96m"
#define ANSI_COLOR_WHITE    "\x1b[97m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static char *logLevelConsoleColors[] = {
        ANSI_COLOR_WHITE,
        ANSI_COLOR_CYAN,
        ANSI_COLOR_YELLOW,
        ANSI_COLOR_RED,
        ANSI_COLOR_MAGENTA
};

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

typedef struct {
    char *filePath;
    volatile NLoggerMode mode;
    NMutex logMutex;
} *NLogger;

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
    uint8_t *elements;
    uint64_t count;
    uint64_t capacity;
    size_t typeSize;
} NList;


extern NList n_list_create(size_t typeSize, uint64_t capacity);
extern void n_list_destroy(NList list);
extern void n_list_add(NList *list, void* value);
extern void n_list_set(NList *list, uint64_t index, void *value);
extern void n_list_get(NList list, uint64_t index, void *destination);
extern uint64_t n_list_index_of(NList list, void *value);
extern void n_list_remove_at(NList *list, uint64_t index);
extern bool n_list_remove(NList *list, void *value);
extern void n_list_clear(NList *list);
extern bool n_list_contains(NList list, void *value);

void *i_n_list_get(NList list, uint64_t index);

#define n_list_add_inline(list, Type, value) { Type i_var = value; n_list_add(list, &i_var); }
#define n_list_set_inline(list, index, Type, value) { Type i_var = value; n_list_set(list, index, &i_var); }
#define n_list_get_inline(list, index, Type) (*((Type*)i_n_list_get(list, index)))

// Test
typedef struct {
    NLogger logger;
    atomic_int passedTestCount;
    atomic_int allTestCount;
} *NTestRunner;

extern void n_test_runner_start(NLogger logger);
extern void n_test_runner_finish();

void i_n_test_assert(const char *testName, int32_t testLine, bool condition, const char *format1, const char *format2, ...);

#define n_test_assert_true(value) i_n_test_assert(__func__, __LINE__, value, "%s", "%s", "true", "false")
#define n_test_assert_false(value) i_n_test_assert(__func__, __LINE__, !value, "%s", "%s", "false", "true")

#define i_n_assert_compare(exp, act, condition, format1, format2) i_n_test_assert(__func__, __LINE__, act condition exp, format1, format2, exp, act)
#define n_assert_size_eq(exp, act)      i_n_assert_compare(exp, act, ==,  "%z",  "%z")
#define n_assert_size_greater(exp, act) i_n_assert_compare(exp, act, >, "> %z",  "%z")
#define n_assert_size_lower(exp, act)   i_n_assert_compare(exp, act, <, "< %z",  "%z")

#define n_assert_i32_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%i",  "%i")
#define n_assert_i32_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %i",  "%i")
#define n_assert_i32_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %i",  "%i")

#define n_assert_u32_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%u",  "%u")
#define n_assert_u32_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %u",  "%u")
#define n_assert_u32_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %u",  "%u")

#define n_assert_i64_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%li", "%li")
#define n_assert_i64_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %li", "%li")
#define n_assert_i64_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %li", "%li")

#define n_assert_u64_eq(exp, act)       i_n_assert_compare(exp, act, ==,  "%lu", "%lu")
#define n_assert_u64_greater(exp, act)  i_n_assert_compare(exp, act, >, "> %lu", "%lu")
#define n_assert_u64_lower(exp, act)    i_n_assert_compare(exp, act, <, "< %lu", "%lu")

//Vectors
typedef struct {
    int32_t x, y;
} NVec2i32;
typedef struct {
    int64_t x, y;
} NVec2i64;
typedef struct {
    float x, y;
} NVec2f32;
typedef struct {
    double x, y;
} NVec2f64;
typedef struct {
    int32_t x, y, z;
} NVec3i32;
typedef struct {
    int64_t x, y, z;
} NVec3i64;
typedef struct {
    float x, y, z;
} NVec3f32;
typedef struct {
    double x, y, z;
} NVec3f64;

typedef union {
    NVec2i32 v2i32;
    NVec2f32 v2f32;
    NVec2i64 v2i64;
    NVec2f64 v2f64;

    NVec3i32 v3i32;
    NVec3f32 v3f32;
    NVec3i64 v3i64;
    NVec3f64 v3f64;
} NVecAnything;

// Window
typedef struct {
    const void *handle;
    const char *title;
} NWindow;

extern NWindow n_window_create(const char *title);

#endif
