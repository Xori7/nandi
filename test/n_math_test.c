#include "test/tests.h"

static Bool n_test_rand(void) {
    N_Random rand = n_rand_init(123);
    I32 INT_MIN = -13;
    I32 INT_MAX = 512420;
    F32 FLOAT_MIN = -2.3f;
    F32 FLOAT_MAX = 6.9f;
    I32 a = n_rand_next_i32(&rand, -10, 20);
    I64 b = n_rand_next_i64(&rand, -10, 20);
    F32 c = n_rand_next_f32(&rand, -1.0, 1.0);
    F64 d = n_rand_next_f64(&rand, -1.0, 0.0);
    if (a < INT_MIN || a > INT_MAX) {
        return FALSE;
    }
    if (b < INT_MIN || b > INT_MAX) {
        return FALSE;
    }
    if (c < FLOAT_MIN || c > FLOAT_MAX) {
        return FALSE;
    }
    if (d < FLOAT_MIN || d > FLOAT_MAX) {
        return FALSE;
    }
    return TRUE;
}

Bool n_test_math(void) {
    return n_test_run("TEST_RAND", &n_test_rand);
}
