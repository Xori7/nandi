#if !N_MEMORY_H
#define N_MEMORY_H 1
#include "n_core.h"
#include <stddef.h>

typedef struct N_Allocator N_Allocator;

struct N_Allocator {
    void* (*alloc)(N_Allocator *allocator, size_t size, size_t alignment);
    void  (*free)(N_Allocator *allocator, void *ptr);
};

extern void*  n_alloc_max_align(N_Allocator *allocator, size_t size);
extern void*  n_alloc(N_Allocator *allocator, size_t size, size_t alignment);
extern void   n_free(N_Allocator *allocator, void *ptr);

// Default allocator that uses malloc and free
extern N_Allocator n_malloc_allocator_create();

typedef struct N_ArenaAllocator N_ArenaAllocator;

// Creates new arena allocator
// PARAMETERS:
//      size - amount of bytes available for allocations
N_ArenaAllocator*   n_arena_allocator_create(N_Allocator *allocator, size_t size);
extern void         n_arena_allocator_destroy(N_ArenaAllocator *arena);
extern void         n_arena_allocator_clear(N_ArenaAllocator *arena);

#endif // !N_MEMORY_H
