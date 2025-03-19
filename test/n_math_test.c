#include "test/tests.h"

static Bool n_test_rand(void) {
    N_Random rand = n_rand_init(123);
    N_UNWRAP(n_debug_print("rand: %d", n_rand_next_i32(&rand, -10, 20)));
    N_UNWRAP(n_debug_print("rand: %d", n_rand_next_i64(&rand, -10, 20)));
    N_UNWRAP(n_debug_print("rand: %f", n_rand_next_f32(&rand, -1.0, 1.0)));
    N_UNWRAP(n_debug_print("rand: %llf", n_rand_next_f64(&rand, -1.0, 0.0)));
    N_UNWRAP(n_debug_print("rand: %d", n_rand_next_i32(&rand, -10, 20)));
    return TRUE;
}

Bool n_test_math(void) {
    return n_test_run("TEST_RAND", &n_test_rand);
}
