#include "nandi/n_test.h"

extern Bool n_test_run(const char *name, Bool(*test_fn)(void)) {
    n_debug_test(name);
    if (test_fn()) {
        n_debug_print("\033[32mPASSED\033[0m", name);
        return TRUE;
    } else {
        n_debug_print("\033[31mFAILED\033[0m", name);
        return FALSE;
    }
}
