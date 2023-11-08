#include "nandi_test.h"

void test_n_context_create_is_returning_valid_context() {
    NContext context = n_context_create();
    n_test_assert_true(testRunner, context);
    n_context_destroy(context);
}

void test_n_context_destroy_makes_context_invalid() {
    NContext context = n_context_create();
    n_context_destroy(context);
    n_test_assert_false(testRunner, context);
}

void test_n_context() {
    test_n_context_create_is_returning_valid_context();
    test_n_context_destroy_makes_context_invalid();
}