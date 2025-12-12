#include "nandi/n_math.h"

static void rand_update(N_Random *random) {
    *random = *random * 6364136223846793005ULL + 1442695040888963407ULL;
}

N_Random n_rand_init(U64 seed) {
    return seed;
}

I32 n_rand_next_i32(N_Random *random, I32 min, I32 max) {
    rand_update(random);
    return min + (I32)(((U32)*random % (1LL << 32)) % (max - min + 1));
}

I64 n_rand_next_i64(N_Random *random, I64 min, I64 max) {
    rand_update(random);
    return min + (I64)((U64)*random % (U64)(max - min + 1));
}

F32 n_rand_next_f32(N_Random *random, F32 min, F32 max) {
    rand_update(random);
    I32 MANTISA_MAX = (1 << 23) - 1;
    F32 v = (F32)n_rand_next_i32(random, 0, MANTISA_MAX) / (F32)MANTISA_MAX;
    return min + ((v) * (max - min));
}

F64 n_rand_next_f64(N_Random *random, F64 min, F64 max) {
    rand_update(random);
    I64 MANTISA_MAX = (1LL << 52) - 1;
    F64 v = (F32)n_rand_next_i64(random, 0, MANTISA_MAX) / (F64)MANTISA_MAX;
    return min + ((v) * (max - min));
}


Bool n_math_equal_vec4_i32(N_Vec4_I32 lhs, N_Vec4_I32 rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}
