#if !N_MATH_H
#define N_MATH_H 1

#include "n_core.h"

typedef U64 N_Random;
N_Random n_rand_init(U64 seed);
I32 n_rand_next_i32(N_Random *random, I32 min, I32 max);
I64 n_rand_next_i64(N_Random *random, I64 min, I64 max);
F32 n_rand_next_f32(N_Random *random, F32 min, F32 max);
F64 n_rand_next_f64(N_Random *random, F64 min, F64 max);


#endif // !N_MATH_H
