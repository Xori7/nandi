#include "nandi/n_core.h"
#include <stdio.h>
#include <stdlib.h>

const char *ERROR_NAMES[] = {
    #define X_ERR(name, code) #name,
    ERROR_LIST
    #undef X_ERR
};

extern const char *n_err_to_str(N_Error error) {
    return ERROR_NAMES[(I32)error];
}

extern void n_unwrap(N_Error error, const char *file, I32 line) {
    N_Error _err = error;
    if (_err != N_ERR_OK)  {
        printf("n_unwrap panic with error: %s at <%s>:%d", n_err_to_str(_err), file, line);
        exit((I32)_err);
    }
}
