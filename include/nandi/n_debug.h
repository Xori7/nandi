#if !N_DEBUG_H
#define N_DEBUG_H 1

#include "n_primitives.h"
#include "n_error.h"

#define N_DEBUG_FILE "./debug/log.txt"

N_Error n_debug_log(const char *fmt, ...);
N_Error n_debug_warn(const char *fmt, ...);
N_Error n_debug_err(const char *fmt, ...);
N_Error n_debug_info(const char *fmt, ...);

#endif // !N_DEBUG_H
