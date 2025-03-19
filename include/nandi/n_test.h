#if !N_TEST_H
#define N_TEST_H 1
#include "n_core.h"

// executes test_fn and logs whether it succeded or failed
extern Bool n_test_run(const char *name, Bool(*test_fn)(void));

#endif // !N_TEST_H
