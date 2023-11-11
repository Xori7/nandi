#include <stdio.h>
#include <stdarg.h>
#include "nandi.h"

extern char *n_cstring_format(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *result = n_internal_cstring_format_args(format, args);
    va_end(args);
    return result;
}

char *n_internal_cstring_format_args(const char *format, va_list args) {
    size_t length;
    char *space;

    length = vsnprintf(NULL, 0, format, args);
    if ((space = n_memory_alloc(length + 1)) != NULL) {
        vsnprintf(space, length + 1, format, args);
        return space;
    }
    return NULL;
}
