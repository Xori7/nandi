#include "nandi_test.h"

bool wasCalled = false;

void mock_on_window_size_changed(NWindow window) {
    wasCalled = true;
}

void test_n_window_create_returns_valid_window() {
    NWindow window = n_window_create("Good one", NULL);
    n_test_assert_true(!memcmp(window->title, "Good one", sizeof *"Good one"));
    n_test_assert_true(window->handle);
    n_window_destroy(window);
}

void test_n_window_set_client_size_changes_window_size() {
    NWindow window = n_window_create("What's next?", NULL);
    NVec2i32 size = {512, 256};
    n_window_set_client_size(window, size);
    n_assert_u32_eq(512, window->size.x);
    n_assert_u32_eq(256, window->size.y);
    n_window_destroy(window);
}

void test_n_window_set_client_size_calls_on_size_changed_func() {
    wasCalled = false;
    NWindow window = n_window_create("Colors of the world...", mock_on_window_size_changed);
    NVec2i32 size = {324, 5832};
    n_window_set_client_size(window, size);
    n_test_assert_true(wasCalled);
    n_window_destroy(window);
}

void test_n_window() {
    test_n_window_create_returns_valid_window();
    test_n_window_set_client_size_changes_window_size();
    test_n_window_set_client_size_calls_on_size_changed_func();
}
