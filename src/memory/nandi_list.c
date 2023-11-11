#include "../nandi.h"
#include <memory.h>

extern NList n_list_create(size_t typeSize, uint64_t capacity) {
    NList list = {
            .elements = n_memory_alloc(typeSize * capacity),
            .count = 0,
            .typeSize = typeSize,
            .capacity = capacity
    };
    return list;
}

extern void n_list_destroy(NList list) {
    n_memory_free(list.elements);
}

void i_resize(NList *list, uint64_t capacity) {
    if (capacity == 0) {
        capacity = 1;
    }
    uint8_t *buffer = n_memory_alloc(capacity * list->typeSize);
    memcpy(buffer, list->elements, list->count * list->typeSize);
    n_memory_free(list->elements);
    list->capacity = capacity;
    list->elements = buffer;
}

extern void n_list_add(NList *list, void* value) {
    if (list->count == list->capacity) {
        i_resize(list, list->capacity * 2);
    }
    memcpy(list->elements + (list->typeSize * list->count), value, list->typeSize);
    list->count++;
}

extern void n_list_set(NList *list, uint64_t index, void *value) {
    memcpy(list->elements + (list->typeSize * index), value, list->typeSize);
}

extern void n_list_get(NList list, uint64_t index, void *variable) {
    memcpy(variable, list.elements + list.typeSize * index, list.typeSize);
}

extern void n_list_remove_at(NList *list, uint64_t index) {
    memmove(list->elements + (list->typeSize * index),
            list->elements + (list->typeSize * (index + 1)),
            list->typeSize * (list->count - (index + 1)));
    list->count--;
}

extern void n_list_clear(NList *list) {
    list->count = 0;
}

extern bool n_list_contains(NList list, void *value) {
    uint8_t *element = list.elements;
    for (element; element < list.elements + (list.typeSize * list.count); element += list.typeSize) {
        if (memcmp(element, value, list.typeSize) == 0) {
            return true;
        }
    }
    return false;
}

void *i_n_list_get(NList list, uint64_t index) {
    return list.elements + list.typeSize * index;
}
