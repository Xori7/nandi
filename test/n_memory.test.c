#include "test/tests.h"

Bool n_test_arena_alloc_after_clear(void) {
    N_ArenaAllocator arena;
    N_UNWRAP(n_arena_allocator_init(12, &arena));

    void *ptr;
    N_UNWRAP(n_alloc(&arena.allocator, 7, 1, &ptr));
    n_arena_allocator_clear(&arena);
    N_Error err = n_alloc(&arena.allocator, 7, 1, &ptr);
    if (err == N_ERR_OK) {
        return TRUE;
    } else {
        n_debug_print("expected: %s", n_err_to_str(N_ERR_OK));
        n_debug_print("  actual: %s", n_err_to_str(err));
        return FALSE;
    }
}

Bool n_test_arena_overflow(void) {
    N_ArenaAllocator arena;
    N_UNWRAP(n_arena_allocator_init(12, &arena));

    void *ptr;
    N_Error err = n_alloc(&arena.allocator, 13, 1, &ptr);

    if (err == N_ERR_ARENA_OVERFLOW) {
        return TRUE;
    } else {
        n_debug_print("expected: %s", n_err_to_str(N_ERR_ARENA_OVERFLOW));
        n_debug_print("  actual: %s", n_err_to_str(err));
        return FALSE;
    }
}

Bool n_test_arena_alloc_alignment(void) {
    N_ArenaAllocator arena;
    N_UNWRAP(n_arena_allocator_init(12, &arena));

    void *ptr1;
    N_UNWRAP(n_alloc(&arena.allocator, 4, 32, &ptr1));

    void *ptr2;
    N_UNWRAP(n_alloc(&arena.allocator, 2, 8, &ptr2));

    const size_t EXPECT = 8;
    size_t offset = (size_t)((U8*)ptr2 - arena.buffer);

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
    result &= n_test_run("ARENA_ALLOC_AFTER_CLEAR", &n_test_arena_alloc_after_clear);
    result &= n_test_run("ARENA_OVERFLOW", &n_test_arena_overflow);
    result &= n_test_run("ARENA_ALLOC_ALIGNMENT", &n_test_arena_alloc_alignment);
    return result;
}
