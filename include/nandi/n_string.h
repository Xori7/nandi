#if !NANDI_NSTRING
#define NANDI_NSTRING 1
#include "n_core.h"
#include "n_memory.h"

typedef struct N_String N_String;

N_String    *n_string_create(N_Allocator *allocator, U64 length);
N_String    *n_string_from_cstring(N_Allocator *allocator, const char *cstring);
U64         n_string_get_length(N_String *string);
char        *n_string_get_buffer(N_String *string);
void        n_string_cat(N_String *lhs, N_String *rhs); // Adds rhs string at the end of lhs string and returns lhs
void        n_string_destroy(N_String *string);

#endif
