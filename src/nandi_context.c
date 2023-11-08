#include "nandi_internal.h"

extern NContext n_context_create(void) {
    NContext context = n_memory_alloc(sizeof *context);
    return context;
}

extern void n_context_destroy(NContext context) {
    n_memory_free(context);
}
