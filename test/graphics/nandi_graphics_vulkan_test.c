#include "../nandi_test.h"

void test_n_vk_graphics_initialize_creates_valid_context() {
    NWindow window = n_window_create("Why I am doing this...", NULL);
    NGraphicsContext graphics = n_graphics_initialize(logger, window);
    n_test_assert_true(graphics.instance);
    n_graphics_cleanup(&graphics);
    n_window_destroy(window);
}

void test_n_vk_graphics() {
    test_n_vk_graphics_initialize_creates_valid_context();
}
