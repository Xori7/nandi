#include <malloc.h>
#include "nandi_internal.h"

extern NContext n_context_create(void) {
    NContext context = malloc(sizeof *context);
    return context;
}

extern void n_context_destroy(NContext context) {
    free(context);
}
