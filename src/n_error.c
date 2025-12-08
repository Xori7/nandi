#include "nandi/n_core.h"
#include "nandi/n_threading.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

static char error[N_MAX_THREAD_COUNT][10240]; //NOTE(xori): 10KB per error message for every thread
                                              
extern N_Result n_error_set(const char *fmt, ...) {
    U32 thread_id = n_threading_current_thread_id();
    va_list args;
    va_start(args, fmt);
    assert(snprintf(error[thread_id], ARRAY_SIZE(error[thread_id]), fmt, args) >= 0 && "failed to set error");
#if N_LOG_ERRORS
    n_debug_err(fmt, fmt, args);
#endif
    va_end(args);
    return N_ERR;
}

extern const char* n_error_get(void) {
    U32 thread_id = n_threading_current_thread_id();
    return error[thread_id];
}

