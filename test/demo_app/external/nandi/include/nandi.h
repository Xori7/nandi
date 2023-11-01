#ifndef NANDI
#define NANDI

#ifndef NANDI_INTERNAL
typedef void *NandiContext;
#endif

extern NandiContext nandi_context_create();
extern void nandi_context_destroy(NandiContext context);

#endif
