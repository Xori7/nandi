#if !NANDI_MEMORY
#define NANDI_MEMORY 1
#include "primitives.h"

typedef struct {
    void *(*alloc)(u64 amount);
    void *(*calloc)(u64 amount);
    void *(*realloc)(void *ptr, u64 amount);
    void (*free)(void *ptr);
} N_Allocator;

N_Allocator     *n_memory_default_allocator_create();
void            n_memory_default_allocator_destroy(N_Allocator *allocator);

#endif
