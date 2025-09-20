#include "test/tests.h"

Bool n_test_arena_alloc_alignment() {
    N_Allocator malloc_allocator = n_malloc_allocator_create();
    N_ArenaAllocator *arena = n_arena_allocator_create(&malloc_allocator, 12);

    void *ptr1 = n_alloc((N_Allocator*)arena, 4, 32);

    void *ptr2 = n_alloc((N_Allocator*)arena, 2, 8);

    const size_t EXPECT = 8;
    size_t offset = (size_t)((U8*)ptr2 - (U8*)ptr1);

    if (offset == EXPECT) {
        return TRUE;
    } else {
        n_debug_print("expected: %lld", EXPECT);
        n_debug_print("  actual: %lld", offset);
        return FALSE;
    }
}

Bool n_test_memory(void) {
    Bool result = TRUE;
    result &= n_test_run("ARENA_ALLOC_ALIGNMENT", &n_test_arena_alloc_alignment);
    return result;
}
