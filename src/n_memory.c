#include "nandi/n_memory.h"
#include <stdlib.h>
#include <stdint.h>

static inline size_t align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

N_Error n_alloc_max_align(N_Allocator *allocator, size_t size, void **out_ptr) {
    return allocator->alloc(allocator, size, sizeof(intmax_t), out_ptr);
}

N_Error n_alloc(N_Allocator *allocator, size_t size, size_t alignment, void **out_ptr) {
    return allocator->alloc(allocator, size, alignment, out_ptr);
}

void n_free(N_Allocator *allocator, void *ptr) {
    allocator->free(allocator, ptr);
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
    free(ptr);
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
    N_DefaultAllocator alloc = {0};
    n_default_allocator_init(&alloc);
    void *buffer = NULL;
    N_Error err = n_alloc_max_align(&alloc.allocator, size, &buffer);
    if (err != N_ERR_OK) {
        return err;
    }

    *out_arena = (N_ArenaAllocator) {
        .allocator = {
            .alloc = &n_arena_allocator_alloc,
            .free = &n_arena_allocator_free
        },
        .buffer = buffer,
        .offset = 0,
        .size = size
    };
    return N_ERR_OK;
}

void n_arena_allocator_deinit(N_ArenaAllocator *arena) {
    free(arena->buffer);
    *arena = (N_ArenaAllocator) {0};
}

void n_arena_allocator_clear(N_ArenaAllocator *arena) {
    arena->offset = 0;
}

