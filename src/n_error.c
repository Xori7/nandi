#include "nandi/n_core.h"
#include <stdio.h>
#include <stdlib.h>

extern const char *n_err_to_str(N_Error error) {
    switch (error) {
        #define X_ERR(name, code) case name: return #name;
        ERROR_LIST
        #undef X_ERR
        default: return "ERROR_UNKNOWN";
    }
}

extern void n_unwrap(N_Error error, const char *file, I32 line) {
    N_Error _err = error;
    if (_err != N_ERR_OK)  {
        printf("n_unwrap panic with error: %s at <%s>:%d\n", n_err_to_str(_err), file, line);
        exit((I32)_err);
    }
}
