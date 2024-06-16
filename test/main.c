#include "stdio.h"
#include "nandi/nandi.h"

int main() {
    N_Allocator *allocator = n_memory_default_allocator_create();
    allocator->alloc(5);
}
