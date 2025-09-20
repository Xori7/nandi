#include "nandi/n_memory.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

static inline size_t align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

void* n_alloc_max_align(N_Allocator *allocator, size_t size) {
    return allocator->alloc(allocator, size, sizeof(intmax_t));
}

void* n_alloc(N_Allocator *allocator, size_t size, size_t alignment) {
    return allocator->alloc(allocator, size, alignment);
}

void n_free(N_Allocator *allocator, void *ptr) {
    allocator->free(allocator, ptr);
}

static inline void* n_default_allocator_alloc(N_Allocator *allocator, size_t size, size_t alignment) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

static inline void n_default_allocator_free(N_Allocator *allocator, void *ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

extern N_Allocator n_malloc_allocator_create() {
    return (N_Allocator) {
        .alloc = &n_default_allocator_alloc,
        .free = &n_default_allocator_free,
    };
}

struct N_ArenaAllocator {
    N_Allocator allocator;
    size_t offset;
    size_t size;
    U8 buffer[];
};

static inline void* n_arena_allocator_alloc(N_Allocator *allocator, size_t size, size_t alignment) {
    assert(allocator != NULL);
    N_ArenaAllocator *arena = (N_ArenaAllocator*)allocator;

    size_t new_ptr = align_up(arena->offset, alignment);
    size_t new_offset = new_ptr + size;
    if (new_offset > arena->size) {
        assert(FALSE && "TODO(xori): handle arena allocator overflow!");
    } else {
        arena->offset = new_offset;
        return arena->buffer + new_ptr;
    }
}

static inline void n_arena_allocator_free(N_Allocator *allocator, void *ptr) { 
    assert(FALSE && "Cannot free arena allocation!");
}

N_ArenaAllocator* n_arena_allocator_create(N_Allocator *allocator, size_t size) {
    N_ArenaAllocator *arena = n_alloc_max_align(allocator, sizeof(N_ArenaAllocator) + size);
    if (arena == NULL) {
        return NULL;
    }

    *arena = (N_ArenaAllocator) {
        .allocator = {
            .alloc = &n_arena_allocator_alloc,
            .free = &n_arena_allocator_free
        },
        .offset = 0,
        .size = size,
    };
    return arena;
}

void n_arena_allocator_destroy(N_ArenaAllocator *arena) {
    free(arena);
}

void n_arena_allocator_clear(N_ArenaAllocator *arena) {
    arena->offset = 0;
    //TODO(xori): add memset in order to reset buffer to zero
}

