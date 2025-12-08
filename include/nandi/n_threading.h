#if !N_THREADING_H
#define N_THREADING_H 1

#include "nandi/n_core.h"

#define N_MAX_THREAD_COUNT 256

extern U32 n_threading_current_thread_id(void);

#endif // N_THREADING_H
