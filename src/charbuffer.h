#pragma once

#include "main.h"

struct charbuffer {
    uint32_t size;
    uint32_t capacity;

    char* buffer;
};


CharBuffer* charbuffer_create ();

void charbuffer_destroy (CharBuffer* cb);

void charbuffer_clear (CharBuffer* cb);


void charbuffer_achar (CharBuffer* cb, char ch);

void charbuffer_achars (CharBuffer* cb, CharBuffer* src);

void charbuffer_astr (CharBuffer* cb, const char* str);


void charbuffer_ichar (CharBuffer* cb, char ch, uint32_t i);

void charbuffer_ichars (CharBuffer* cb, CharBuffer* src, uint32_t i);

void charbuffer_istr (CharBuffer* cb, const char* str, uint32_t i);


uint32_t charbuffer_get (CharBuffer* cb, uint32_t i);

void charbuffer_get_substr (CharBuffer* cb, CharBuffer* dst, uint32_t i, uint32_t j);

void charbuffer_get_prefix (CharBuffer* cb, CharBuffer* dst, uint32_t i);

void charbuffer_get_suffix (CharBuffer* cb, CharBuffer* dst, uint32_t i);

void charbuffer_copy (CharBuffer* cb, CharBuffer* dst);


void charbuffer_rm_char (CharBuffer* cb, uint32_t i);

void charbuffer_rm_prefix (CharBuffer* cb, uint32_t i);

void charbuffer_rm_suffix (CharBuffer* cb, uint32_t i);

void charbuffer_rm_substr (CharBuffer* cb, uint32_t i, uint32_t j);


void charbuffer_read (CharBuffer* cb, FILE* file);

bool charbuffer_read_line (CharBuffer* cb, FILE* file);

void charbuffer_write (CharBuffer* cb, FILE* file);
