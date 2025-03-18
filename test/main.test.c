#include "nandi/nandi.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    N_DefaultAllocator allocator;
    n_default_allocator_init(&allocator);

    N_OK_PANIC(n_debug_log("something"));
    N_OK_PANIC(n_debug_log("something"));
    for (I32 i = 0; i < 100; i++) {
        N_OK_PANIC(n_debug_err("Test: %d", i));
        N_OK_PANIC(n_debug_info("ur mom"));
    }
    N_OK_PANIC(n_debug_warn("end"));
}
