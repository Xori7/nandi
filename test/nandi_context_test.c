#include "nandi_test.h"

void test_n_context_create_returns_valid_context() {
    NContext context = n_context_create();
    n_test_assert_true(testRunner, context);
    n_context_destroy(context);
}

void test_n_context() {
    test_n_context_create_returns_valid_context();
}