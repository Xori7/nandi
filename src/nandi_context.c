#include <malloc.h>
#include "nandi_internal.h"

extern NandiContext nandi_context_create(void) {
    NandiContext context = malloc(sizeof *context);
    return context;
}

extern void nandi_context_destroy(NandiContext context) {
    free(context);
}
