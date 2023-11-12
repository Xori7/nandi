#include "nandi_test.h"

void test_n_window_create_has_valid_handle() {
    NWindow window = n_window_create("Good one");
    n_test_assert_true(!memcmp(window.title, "Good one", sizeof *"Good one"));
}

void test_n_window() {
    test_n_window_create_has_valid_handle();
}
