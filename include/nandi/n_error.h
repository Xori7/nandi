#if !N_ERROR_H
#define N_ERROR_H 1

typedef enum {
    N_ERR_OK = 0,
    N_ERR_OUT_OF_MEMORY = 1,
    N_ERR_ALLOCATOR_NOT_INITIALIZED = 2,
} N_Error;

#endif // !N_ERROR_H
