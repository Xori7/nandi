#if !N_ERROR_H
#define N_ERROR_H 1

#include "n_primitives.h"

typedef enum {
    N_ERR_OK = 0,
    N_ERR_OUT_OF_MEMORY = 1,
    N_ERR_ALLOCATOR_NOT_INITIALIZED = 2,
} N_Error;

typedef struct {
    const char *message;
    U64 error_code;
} N_ErrorContext;

#define N_OK(call) do { \
    int result = (call); \
    if (result < 0) return result; \
} while (0)

#endif // !N_ERROR_H
