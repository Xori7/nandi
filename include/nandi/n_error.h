#if !N_ERROR_H
#define N_ERROR_H 1

#include "n_primitives.h"

// NOTE(xori): error categories
#define N_ERRC_MEMORY        (1 << 10)
#define N_ERRC_FILE_SYSTEM   (1 << 11)

typedef enum {
    N_ERR_OK                        = 0,
    N_ERR_OUT_OF_MEMORY             = N_ERRC_MEMORY | 1,
    N_ERR_ALLOCATOR_NOT_INITIALIZED = N_ERRC_MEMORY | 2,
    N_ERR_FILE_OPEN                 = N_ERRC_FILE_SYSTEM | 1,
    N_ERR_FILE_CLOSE                = N_ERRC_FILE_SYSTEM | 2,
    N_ERR_FILE_WRITE                = N_ERRC_FILE_SYSTEM | 3,
    N_ERR_PRINTF_FAIL               = N_ERRC_FILE_SYSTEM | 4, // NOTE(xori): maybe move that to some more apropriate category?
    N_ERR_SPRFTIME_FAIL             = N_ERRC_FILE_SYSTEM | 5,
} N_Error;

#define N_OK(call) do { \
    N_Error result = (call); \
    if (result != N_ERR_OK) return result; \
} while (0)

#define N_OK_PANIC(call) do { \
    N_Error result = (call); \
    if (result != N_ERR_OK) exit((I32)result); \
} while (0)

#endif // !N_ERROR_H
