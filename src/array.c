#include "array.h"


static void array_expand (Array* array) {
    uint32_t new_capacity = array->capacity * 2;
    void** new_array = calloc(new_capacity, sizeof(void*));

    for (int i = 0; i < array->size; ++i)
        new_array[i] = array->data[i];

    free(array->data);
    array->data = new_array;
    array->capacity = new_capacity;
}

Array* array_create () {
    return array_create_capacity(16);
}

Array* array_create_capacity (uint32_t capacity) {
    Array* array = malloc(sizeof(Array));
    array->data = calloc(capacity, sizeof(void*));
    array->size = 0;
    array->capacity = capacity;

    return array;
}

void array_destroy (Array* array) {
    free(array->data);
    free(array);
}

void array_destroy_callback (Array* array, array_callback callback) {
    if (callback != NULL)
        for (int i = 0; i < array->size; ++i)
            callback(array->data[i]);
    free(array->data);
    free(array);
}

void array_add (Array* array, void* item) {
    array_push(array, item);
}

void array_clear (Array* array) {
    array->size = 0;
}

void array_push (Array* array, void* item) {
    if (array->size >= array->capacity) array_expand(array);

    array->data[array->size] = item;
    array->size++;
}

void* array_pop  (Array* array) {
    if (array->size == 0) return NULL;

    array->size--;
    return array->data[array->size];
}

void* array_get (Array* array, int32_t index) {
    if (index < 0 || index >= array->size) return NULL;

    return array->data[index];
}

void* array_set (Array* array, int32_t index, void* item) {
    if (index < 0) return NULL;
    if (index >= array->size) {
        array_add(array, item);
        return NULL;
    }

    void* r = array->data[index];
    array->data[index] = item;
    return r;
}

void array_insert (Array* array, int32_t index, void* item) {
    if (index < 0) return;
    if (index >= array->size) {
        array_push(array, item);
        return;
    }

    if (array->size >= array->capacity) array_expand(array);

    for (int i = array->size; i >= index; i--) {
        array->data[i+1] = array->data[i];
    }

    array->data[index] = item;
    array->size++;
}

void* array_remove (Array* array, int32_t index) {
    if (index < 0 || index >= array->size) return NULL;

    void* r = array->data[index];
    for (int i = index; i < array->size - 1; ++i) {
        array->data[i] = array->data[i+1];
    }

    array->size--;

    return r;
}
