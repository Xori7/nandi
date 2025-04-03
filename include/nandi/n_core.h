#if !N_CORE_H
#define N_CORE_H 1

// # TYPES #
#include <stdint.h>
#include <stddef.h>

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

// # ERROR #

#define ERROR_LIST \
    X_ERR(N_ERR_OK                          , 0) \
    X_ERR(N_ERR_OUT_OF_MEMORY               , 1) \
    X_ERR(N_ERR_ALLOCATOR_NOT_INITIALIZED   , 2) \
    X_ERR(N_ERR_ARENA_OVERFLOW              , 3) \
    X_ERR(N_ERR_FILE_OPEN                   , 1000) \
    X_ERR(N_ERR_FILE_CLOSE                  , 1004) \
    X_ERR(N_ERR_FILE_WRITE                  , 1005) \
    X_ERR(N_ERR_PRINTF_FAIL                 , 1006) \
    X_ERR(N_ERR_SPRFTIME_FAIL               , 1007) \


typedef enum {
    #define X_ERR(name, code) name = code,
    ERROR_LIST
    #undef X_ERR
} N_Error;

// returns error name string
extern const char *n_err_to_str(N_Error error);
extern void n_unwrap(N_Error error, const char *file, I32 line);

#define N_OK(call) do {                     \
    N_Error result = (call);                \
    if (result != N_ERR_OK) return result;  \
} while (0)

#define N_UNWRAP(_call) n_unwrap((_call), __FILE__, __LINE__)

// # DEBUG #
#define N_DEBUG_FILE "./debug/log.txt"

extern void n_debug_log(const char *fmt, ...);
extern void n_debug_warn(const char *fmt, ...);
extern void n_debug_err(const char *fmt, ...);
extern void n_debug_info(const char *fmt, ...);
extern void n_debug_test(const char *fmt, ...);
extern void n_debug_print(const char *fmt, ...);

#endif // !N_CORE_H
