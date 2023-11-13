#include "nandi_test.h"

void test_n_window_create_returns_valid_window() {
    NWindow window = n_window_create("Good one");
    n_test_assert_true(!memcmp(window->title, "Good one", sizeof *"Good one"));
    n_test_assert_true(window->handle);
    n_window_destroy(window);
}

void test_n_window() {
    test_n_window_create_returns_valid_window();
}
