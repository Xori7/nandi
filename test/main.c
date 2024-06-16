#include "nandi/nandi.h"
#include <stdio.h>

int main() {
    N_Allocator *allocator = n_memory_default_allocator_create();
    N_String *string = n_string_from_cstring(allocator, "Ksd");
    N_String *string2 = n_string_from_cstring(allocator, "1234");

    n_string_cat(string, string2);
    printf("%llu", n_string_get_length(string));
    printf("%s", n_string_get_buffer(string));
}
