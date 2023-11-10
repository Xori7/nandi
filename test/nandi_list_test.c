#include "nandi_test.h"

void test_n_list_created_empty() {
    NList list = n_list_create(char, 10);
    n_test_assert_int64_equal(testRunner, 0, list.count);
    n_list_destroy(list);
}

void test_n_list_created_with_correct_capacity(int capacity) {
    NList list = n_list_create(char, capacity);
    n_test_assert_int64_equal(testRunner, capacity, list.capacity);
    n_list_destroy(list);
}

void test_n_list_add_increments_count() {
    NList list = n_list_create(char, 10);
    n_list_add(char, list, 10);
    n_test_assert_int64_equal(testRunner, 1, list.count);
    n_list_add(char, list, 10);
    n_test_assert_int64_equal(testRunner, 2, list.count);
    n_list_destroy(list);
}

void test_n_list_add_sets_element_value(int32_t value) {
    NList list = n_list_create(int32_t, 10);
    n_list_add(int32_t, list, value);
    n_test_assert_int32_equal(testRunner, value, n_list_get(int32_t, list, 0));
    n_list_destroy(list);
}

void test_n_list() {
    test_n_list_created_empty();
    test_n_list_created_with_correct_capacity(10);
    test_n_list_created_with_correct_capacity(17);
    test_n_list_add_increments_count();
    test_n_list_add_sets_element_value(7);
    test_n_list_add_sets_element_value(420);
}