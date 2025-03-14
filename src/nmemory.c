#include "nandi/n_memory.h"
#include <assert.h>
#include <stdlib.h>


static inline size_t align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

static inline N_Error n_default_allocator_alloc(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr) {
    *out_ptr = _aligned_malloc(size, alignment);
    if (*out_ptr == NULL) {
        return N_ERR_OUT_OF_MEMORY;
    } else {
        return N_ERR_OK;
    }
}

static inline void n_default_allocator_free(N_Allocator *allocator, void *ptr) {
    return free(ptr);
}

void n_default_allocator_init(N_DefaultAllocator *out_allocator) {
    out_allocator->allocator = (N_Allocator) {
        .alloc = &n_default_allocator_alloc,
        .free = &n_default_allocator_free,
    };
}

static inline N_Error n_arena_allocator_alloc(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr) {
    N_ArenaAllocator *arena = (N_ArenaAllocator*)allocator;
    if (arena->buffer != NULL) {
        return N_ERR_ALLOCATOR_NOT_INITIALIZED;
    } 

    size_t new_offset = align_up(arena->offset, alignment) + size;
    if (new_offset > arena->size) {
        return N_ERR_OUT_OF_MEMORY;
    } else {
        *out_ptr = arena->buffer + new_offset;
        return N_ERR_OK;
    }
}

static inline void n_arena_allocator_free(N_Allocator *allocator, void *ptr) { 
}

N_Error n_arena_allocator_init(size_t size, N_ArenaAllocator *out_arena) {
    *out_arena = (N_ArenaAllocator) {
        .allocator = {
            .alloc = &n_arena_allocator_alloc,
            .free = &n_arena_allocator_free
        },
        .buffer = malloc(size),
        .offset = 0,
        .size = size
    };
    if (out_arena->buffer == NULL) {
        return N_ERR_OUT_OF_MEMORY;
    } else {
        return N_ERR_OK;
    }
}

void n_arena_allocator_deinit(N_ArenaAllocator *arena) {
    free(arena->buffer);
    *arena = (N_ArenaAllocator) {0};
}

void n_arena_allocator_clear(N_ArenaAllocator *arena) {
    arena->offset = 0;
}

