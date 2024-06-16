#include "nandi/nstring.h"
#include <string.h>
#include <assert.h>

struct N_String {
    char *buffer;
    u64 length;
    N_Allocator *allocator;
};

N_String *n_string_create(N_Allocator *allocator, u64 length) {
    assert(allocator != NULL && "allocator can't be NULL");
    N_String *string = allocator->alloc(sizeof(*string));
    string->buffer = allocator->alloc(sizeof(*string->buffer) * length);
    string->length = length;
    string->allocator = allocator;
    return string;
}

N_String *n_string_from_cstring(N_Allocator *allocator, const char *cstring) {
    assert(allocator != NULL && "allocator can't be NULL");
    assert(cstring != NULL && "cstring can't be NULL");
    N_String *string = n_string_create(allocator, strlen(cstring));
    memcpy(string->buffer, cstring, string->length);
    return string;
}

u64 n_string_get_length(N_String *string) {
    assert(string != NULL && "string can't be NULL");
    return string->length;
}

char *n_string_get_buffer(N_String *string) {
    assert(string != NULL && "string can't be NULL");
    return string->buffer;
}

void n_string_resize(N_String *string, u64 length) {
    assert(string != NULL && "string can't be NULL");
    string->buffer = string->allocator->realloc(string->buffer, length);
    assert(string->buffer != NULL);
    string->length = length;
}

void n_string_cat(N_String *lhs, N_String *rhs) {
    u64 lhsLength = n_string_get_length(lhs);
    u64 rhsLength = n_string_get_length(rhs);
    n_string_resize(lhs, lhsLength + rhsLength);
    memcpy(lhs->buffer + lhsLength, rhs->buffer, rhsLength);
}

void n_string_destroy(N_String *string) {
    string->allocator->free(string->buffer);
    string->allocator->free(string);
}
