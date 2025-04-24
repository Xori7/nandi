#if !N_MEMORY_H
#define N_MEMORY_H 1
#include "n_core.h"
#include <stddef.h>

typedef struct N_Allocator N_Allocator;

struct N_Allocator {
    N_Error (*alloc)(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr);
    void (*free)(N_Allocator *allocator, void *ptr);
};

extern N_Error  n_alloc_max_align(N_Allocator *allocator, size_t size, void **out_ptr);
extern N_Error  n_alloc(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr);
extern void     n_free(N_Allocator *allocator, void *ptr);

// Default allocator that uses malloc and free
typedef struct {
    N_Allocator allocator;
} N_DefaultAllocator;

extern void n_default_allocator_init(N_DefaultAllocator *out_allocator);

typedef struct {
    N_Allocator allocator;
    U8* buffer;
    size_t offset;
    size_t size;
} N_ArenaAllocator;

// Creates new arena allocator
// PARAMETERS:
//      size - amount of bytes available for allocations
//      out_arena - target arena location ptr
// RETURNS:
//  - N_ERR_OK
//  - N_ERR_OUT_OF_MEMORY
extern N_Error n_arena_allocator_init(U64 size, N_ArenaAllocator *out_arena);
extern void    n_arena_allocator_deinit(N_ArenaAllocator *arena);
extern void    n_arena_allocator_clear(N_ArenaAllocator *arena);

#endif // !N_MEMORY_H
