#ifdef _WIN32

#include "nandi/n_threading.h"

extern U32 n_threading_current_thread_id(void) {
    return 0;
}

#endif // _WIN32
