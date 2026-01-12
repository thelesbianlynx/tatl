#pragma once

#include "main.h"

typedef struct rope_node Node;

typedef bool (*rope_foreach_fn) (uint32_t i, uint32_t ch, void* data);


struct point {
    int32_t row, col;
};

struct rope {
    Node* node;
};


Rope* rope_create (IntBuffer* src);

Rope* rope_copy (Rope* rope);

void rope_destroy (Rope* rope);


uint32_t rope_len (Rope* rope);

uint32_t rope_lines (Rope* rope);


uint32_t rope_get_char (Rope* rope);


Rope* rope_prefix (Rope* rope, uint32_t i);

Rope* rope_suffix (Rope* rope, uint32_t i);

Rope* rope_substr (Rope* rope, uint32_t i, uint32_t j);

Rope* rope_append (Rope* a, Rope* b);


Point rope_index_to_point (Rope* rope, int32_t index);

uint32_t rope_point_to_index (Rope* rope, Point point);


void rope_foreach (Rope* rope, rope_foreach_fn fn, void* data);

void rope_foreach_prefix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data);

void rope_foreach_suffix (Rope* rope, uint32_t i, rope_foreach_fn fn, void* data);

void rope_foreach_substr (Rope* rope, uint32_t i, uint32_t j, rope_foreach_fn fn, void* data);


void rope_print (Rope* rope);
