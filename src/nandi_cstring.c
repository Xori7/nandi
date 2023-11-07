#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include "nandi_internal.h"

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

    length = vsnprintf(0, 0, format, args);
    if ((space = malloc(length + 1)) != 0) {
        vsnprintf(space, length + 1, format, args);
        return space;
    }
    return NULL;
}
