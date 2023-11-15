#include "../nandi_test.h"

void test_n_list_create_with_correct_values(size_t typeSize, uint64_t capacity) {
    NList list = n_list_create(typeSize, capacity);
    n_assert_u64_eq(0, list.count);
    n_assert_u64_eq(capacity, list.capacity);
    n_assert_size_eq(typeSize, list.typeSize);
    n_test_assert_true(list.elements);
    n_list_destroy(list);
}

void test_n_list_create_filled_with_correct_values(size_t typeSize, uint64_t count) {
    NList list = n_list_create_filled(typeSize, count);
    n_assert_u64_eq(count, list.count);
    n_assert_u64_eq(count, list.capacity);
    n_assert_size_eq(typeSize, list.typeSize);
    n_test_assert_true(list.elements);
    void *zeroBuffer = n_memory_alloc(typeSize * count);
    memset(zeroBuffer, 0, typeSize * count);
    n_test_assert_false(memcmp(list.elements, zeroBuffer, typeSize * count));
    n_memory_free(zeroBuffer);
    n_list_destroy(list);
}

void test_n_list_add_increments_count() {
    NList list = n_list_create(sizeof(int32_t), 10);
    int32_t value = 12;
    n_list_add(&list, &value);
    n_assert_u64_eq(1, list.count);
    n_list_add_inline(&list, int32_t, 7);
    n_assert_u64_eq(2, list.count);
    n_list_destroy(list);
}

void test_n_list_get_returns_added_values() {
    NList list = n_list_create(sizeof(int32_t), 10);
    n_list_add_inline(&list, int32_t, 44);
    n_list_add_inline(&list, int32_t, -7);

    int32_t value = -1;
    n_list_get(list, 0, &value);
    n_assert_i32_eq(44, value);
    n_assert_i32_eq(-7, n_list_get_inline(list, 1, int32_t));

    n_list_destroy(list);
}

void test_n_list_add_more_elements_than_capacity_doubles_capacity() {
    const char *value = "Life is just a sinusoidal fractal...";
    NList list = n_list_create(sizeof(const char *), 1);
    n_list_add_inline(&list, const char*, "it really is...");
    n_assert_u64_eq(1, list.capacity);
    n_list_add(&list, &value);
    n_assert_u64_eq(2, list.capacity);
    n_list_add(&list, &value);
    n_assert_u64_eq(4, list.capacity);
    n_list_destroy(list);
}

void test_n_list_set_correctly_changes_value() {
    NList list = n_list_create(sizeof(int16_t), 2);
    n_list_add_inline(&list, int16_t, 2);
    n_list_add_inline(&list, int16_t, -10);
    int16_t value = 15420;
    n_list_set(&list, 0, &value);
    n_list_set_inline(&list, 1, int16_t, -1234);
    n_assert_i64_eq(15420, n_list_get_inline(list, 0, int16_t));
    n_assert_i64_eq(-1234, n_list_get_inline(list, 1, int16_t));
    n_list_destroy(list);
}

void test_n_list_remove_at_shifts_subsequent_elements_back() {
    NList list = n_list_create(sizeof(uint32_t), 0);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    n_list_add_inline(&list, uint32_t, 30);
    n_list_add_inline(&list, uint32_t, 123121);
    n_list_remove_at(&list, 1);
    n_assert_u32_eq(15, n_list_get_inline(list, 0, uint32_t));
    n_assert_u32_eq(30, n_list_get_inline(list, 1, uint32_t));
    n_assert_u32_eq(123121, n_list_get_inline(list, 2, uint32_t));

    n_list_destroy(list);
}

void test_n_list_remove_at_decrements_count() {
    NList list = n_list_create(sizeof(uint32_t), 2);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    n_list_remove_at(&list, 0);
    n_assert_u64_eq(1, list.count);
    n_list_remove_at(&list, 0);
    n_assert_u64_eq(0, list.count);
    n_list_destroy(list);
}

void test_n_list_remove_returns_true_when_element_exists_and_false_when_it_does_not() {
    NList list = n_list_create(sizeof(uint32_t), 4);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    n_list_add_inline(&list, uint32_t, 30);
    n_list_add_inline(&list, uint32_t, 123121);
    uint32_t value = 3;
    n_test_assert_false(n_list_remove(&list, &value));
    value = 30;
    n_test_assert_true(n_list_remove(&list, &value));
    n_list_destroy(list);
}

void test_n_list_remove_shifts_subsequent_elements_back() {
    NList list = n_list_create(sizeof(uint32_t), 0);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    n_list_add_inline(&list, uint32_t, 30);
    n_list_add_inline(&list, uint32_t, 123121);
    uint32_t value = 4;
    n_list_remove(&list, &value);
    n_assert_u32_eq(15, n_list_get_inline(list, 0, uint32_t));
    n_assert_u32_eq(30, n_list_get_inline(list, 1, uint32_t));
    n_assert_u32_eq(123121, n_list_get_inline(list, 2, uint32_t));

    n_list_destroy(list);
}

void test_n_list_remove_decrements_count() {
    NList list = n_list_create(sizeof(uint32_t), 2);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    uint32_t value = 15;
    n_list_remove(&list, &value);
    n_assert_u64_eq(1, list.count);
    value = 4;
    n_list_remove(&list, &value);
    n_assert_u64_eq(0, list.count);
    n_list_destroy(list);
}

void test_n_list_clear_sets_count_to_zero() {
    NList list = n_list_create(sizeof(uint32_t), 2);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    n_list_clear(&list);
    n_assert_u64_eq(0, list.count);
    n_list_destroy(list);
}

void test_n_list_contains_returns_true_when_value_was_added_to_list() {
    NList list = n_list_create(sizeof(uint32_t), 2);
    n_list_add_inline(&list, uint32_t, 15);
    n_list_add_inline(&list, uint32_t, 4);
    uint32_t value = 3;
    n_test_assert_false(n_list_contains(list, &value));
    value = 64;
    n_test_assert_false(n_list_contains(list, &value));
    value = 4;
    n_test_assert_true(n_list_contains(list, &value));
    value = 15;
    n_test_assert_true(n_list_contains(list, &value));
    n_list_destroy(list);
}

void test_n_list_index_of_returns_index_of_passed_element() {
    NList list = n_list_create(sizeof(char *), 4);
    char *text = "He just don't sleep :o";
    n_list_add_inline(&list, char*, "I don't know why");
    n_list_add_inline(&list, char*, "Or do I?");
    n_list_add(&list, &text);
    n_list_add_inline(&list, char*, "Btw, this is so scary that you can forget to put '&' before pointer variable in list_add argument (＠_＠;)");

    n_assert_u64_eq(2, n_list_index_of(list, &text));

    n_list_destroy(list);
}

void test_n_list() {
    test_n_list_create_with_correct_values(sizeof(int32_t), 10);
    test_n_list_create_with_correct_values(sizeof(int64_t), 420);
    test_n_list_create_filled_with_correct_values(sizeof(int32_t), 10);
    test_n_list_create_filled_with_correct_values(sizeof(int64_t), 420);
    test_n_list_add_increments_count();
    test_n_list_get_returns_added_values();
    test_n_list_add_more_elements_than_capacity_doubles_capacity();
    test_n_list_set_correctly_changes_value();
    test_n_list_remove_at_shifts_subsequent_elements_back();
    test_n_list_remove_at_decrements_count();
    test_n_list_remove_returns_true_when_element_exists_and_false_when_it_does_not();
    test_n_list_remove_shifts_subsequent_elements_back();
    test_n_list_remove_decrements_count();
    test_n_list_clear_sets_count_to_zero();
    test_n_list_contains_returns_true_when_value_was_added_to_list();
    test_n_list_index_of_returns_index_of_passed_element();
}