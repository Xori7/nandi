#if !N_CORE_H
#define N_CORE_H 1

#define N_LOG_ERRORS 1

// # TYPES #
#include <stdint.h>

typedef int8_t      I8;
typedef int16_t     I16;
typedef int32_t     I32;
typedef int64_t     I64;

typedef uint8_t     U8;
typedef uint16_t    U16;
typedef uint32_t    U32;
typedef uint64_t    U64;

typedef float       F32;
typedef double      F64;

typedef U8          Bool;
#define TRUE ((U8)1)
#define FALSE ((U8)0)

typedef enum {
    N_OK = 1,
    N_ERR = 0
} N_Result;

typedef _Atomic(I8)   AtomicI8;
typedef _Atomic(I16)  AtomicI16;
typedef _Atomic(I32)  AtomicI32;
typedef _Atomic(I64)  AtomicI64;

typedef _Atomic(U8)   AtomicU8;
typedef _Atomic(U16)  AtomicU16;
typedef _Atomic(U32)  AtomicU32;
typedef _Atomic(U64)  AtomicU64;

typedef _Atomic(F32)  AtomicF32;
typedef _Atomic(F64)  AtomicF64;

typedef _Atomic(Bool) AtomicBool;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// # ERROR #

// Returns: always N_ERR, which can be used to return with error_set, e.g. return n_error_set("error message");
extern N_Result     n_error_set(const char *fmt, ...);

// Returns: last error on the current thread set by n_error_set() function
extern const char*  n_error_get(void);

// # DEBUG #
#define N_DEBUG_FILE "./debug/log.txt"

extern void n_debug_log(const char *fmt, ...);
extern void n_debug_warn(const char *fmt, ...);
extern void n_debug_err(const char *fmt, ...);
extern void n_debug_info(const char *fmt, ...);
extern void n_debug_test(const char *fmt, ...);
extern void n_debug_print(const char *fmt, ...);

// returns current application time in seconds
extern F64 n_debug_time(void);

#define N_DEBUG_MESURE(name, code)  \
{                                   \
    F64 start = n_debug_time();     \
    {code}                          \
    F64 end = n_debug_time();       \
    F64 time = end - start;         \
    n_debug_info("N_DEBUG_MESURE %s: %.4fms", name, time * 1000);\
}

#endif // !N_CORE_H
