#if !N_DEBUG_H
#define N_DEBUG_H 1

#include "n_primitives.h"
#include "n_error.h"

#define N_DEBUG_LOG(msg) n_debug_log(msg, __FILE__, __LINE__);
#define N_DEBUG_WARN(msg) n_debug_warn(msg, __FILE__, __LINE__);
#define N_DEBUG_ERR(msg) n_debug_err(msg, __FILE__, __LINE__);

#define N_DEBUG_FILE "./debug.log"

N_Error n_debug_log(const char *msg, const char *file, U32 line);
N_Error n_debug_warn(const char *msg, const char *file, U32 line);
N_Error n_debug_err(const char *msg, const char *file, U32 line);

#endif // !N_DEBUG_H
