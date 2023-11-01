#include "nandi.h"

int main() {
    NandiContext context = nandi_context_create();
    nandi_context_destroy(context);
    return 0;
}