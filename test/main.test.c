#include "nandi/nandi.h"
#include "test/tests.h"

int main(void) {
    N_DefaultAllocator allocator;
    n_default_allocator_init(&allocator);
    n_test_memory();
    n_test_math();
    n_test_graphics();
}

