#if !N_MEMORY_H
#define N_MEMORY_H 1
#include "n_primitives.h"
#include "n_error.h"

typedef struct N_Allocator N_Allocator;

struct N_Allocator {
    N_Error (*alloc)(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr);
    void (*free)(N_Allocator *allocator, void *ptr);
};

#define N_ALLOC_MAX_ALIGN(allocator_ptr, size, out_ptr) ((N_IAllocator*)allocator_ptr)->alloc((N_IAllocator*)allocator_ptr, size, _Alignof(max_align_t), out_ptr)
#define N_ALLOC(allocator_ptr, size, alignment, out_ptr) ((N_IAllocator*)allocator_ptr)->alloc((N_IAllocator*)allocator_ptr, size, alignment, out_ptr)
#define N_FREE(allocator_ptr, ptr) ((N_IAllocator*)allocator_ptr)->free((N_IAllocator*)allocator_ptr, ptr)



// Default allocator that uses malloc and free
typedef struct {
    N_Allocator allocator;
} N_DefaultAllocator;

void n_default_allocator_init(N_DefaultAllocator *out_allocator);



typedef struct {
    N_Allocator allocator;
    void* buffer;
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
N_Error n_arena_allocator_init(U64 size, N_ArenaAllocator *out_arena);
void    n_arena_allocator_deinit(N_ArenaAllocator *arena);
void    n_arena_allocator_clear(N_ArenaAllocator *arena);

#endif // !N_MEMORY_H
