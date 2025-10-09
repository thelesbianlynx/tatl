#pragma once

#include "main.h"


struct intbuffer {
    int32_t* data;

    uint32_t size;
    uint32_t capacity;

    // Undo/Redo tracking.
    bool damage;
    int32_t lineno;

};


IntBuffer* intbuffer_create ();

void intbuffer_destroy (IntBuffer* line);


void intbuffer_clear (IntBuffer* line);


void intbuffer_put_char (IntBuffer* line, uint32_t i, int32_t ch);

void intbuffer_put_text (IntBuffer* line, uint32_t i, CharBuffer* src);


void intbuffer_put_substr (IntBuffer* line, uint32_t i, IntBuffer* src, uint32_t j, uint32_t k);

void intbuffer_put_prefix (IntBuffer* line, uint32_t i, IntBuffer* src, uint32_t j);

void intbuffer_put_suffix (IntBuffer* line, uint32_t i, IntBuffer* src, uint32_t j);


void intbuffer_get_substr (IntBuffer* line, uint32_t i, uint32_t j, CharBuffer* dst);

void intbuffer_get_prefix (IntBuffer* line, uint32_t i, CharBuffer* dst);

void intbuffer_get_suffix (IntBuffer* line, uint32_t i, CharBuffer* dst);



void intbuffer_rm_substr (IntBuffer* line, uint32_t i, uint32_t j);

void intbuffer_rm_prefix (IntBuffer* line, uint32_t i);

void intbuffer_rm_suffix (IntBuffer* line, uint32_t i);
