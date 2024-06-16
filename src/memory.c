#include "nandi/memory.h"
#include <assert.h>
#include <stdlib.h>

void *default_allocator_alloc(u64 amount) {
    assert(amount > 0 && "can't allocate 0 bytes!");
    return malloc(amount);
}

void *default_allocator_calloc(u64 amount) {
    assert(amount > 0 && "can't allocate 0 bytes!");
    return calloc(1, amount);
}

void *default_allocator_realloc(void *ptr, u64 amount) {
    assert(ptr != NULL && "can't realloc NULL pointer!");
    assert(amount > 0 && "can't allocate 0 bytes!");
    return realloc(ptr, amount);
}

void default_allocator_free(void *ptr) {
    free(ptr);
}

N_Allocator *n_memory_default_allocator_create() {
    N_Allocator *allocator = malloc(sizeof(*allocator));
    allocator->alloc = default_allocator_alloc;
    allocator->calloc = default_allocator_calloc;
    allocator->realloc = default_allocator_realloc;
    allocator->free = default_allocator_free;
    return allocator;
}

void n_memory_default_allocator_destroy(N_Allocator *allocator) {
    free(allocator);
}
