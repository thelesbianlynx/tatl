#include "intbuffer.h"

#include "charbuffer.h"
#include "codepoint.h"

static
void intbuffer_expand (IntBuffer* buffer) {
    uint32_t new_capacity = buffer->capacity * 2;
    int32_t* new_array = calloc(new_capacity, sizeof(int32_t));

    for (int i = 0; i < buffer->size; i++)
        new_array[i] = buffer->data[i];

    free(buffer->data);
    buffer->data = new_array;
    buffer->capacity = new_capacity;
}


IntBuffer* intbuffer_create () {
    int capacity = 1;
    IntBuffer* buffer = malloc(sizeof(IntBuffer));
    buffer->data = calloc(capacity, sizeof(int32_t));
    buffer->size = 0;
    buffer->capacity = capacity;
    buffer->damage = true;
    buffer->lineno = 0;

    return buffer;
}

void intbuffer_destroy (IntBuffer* buffer) {
    free(buffer->data);
    free(buffer);
}


void intbuffer_clear (IntBuffer* buffer) {
    buffer->size = 0;
}


void intbuffer_put_char (IntBuffer* buffer, uint32_t i, int32_t ch) {
    if (i > buffer->size) i = buffer->size;

    while (buffer->capacity < buffer->size + 1)
        intbuffer_expand(buffer);

    for (int x = buffer->size - 1; x >= (int) i; x--) {
        buffer->data[x+1] = buffer->data[x];
    }

    buffer->size++;
    buffer->data[i] = ch;
    buffer->damage = true;
}

void intbuffer_put_text (IntBuffer* buffer, uint32_t i, CharBuffer* src) {
    for (int n = 0; n < src->size;) {
        uint32_t ch = 0;
        int r = chars_to_codepoint(src->buffer + n, src->size - n, &ch);

        if (r == 0) {
            n += 1;
            continue;
        }

        intbuffer_put_char(buffer, i++, ch);
        n += r;
    }
}


void intbuffer_put_substr (IntBuffer* buffer, uint32_t i, IntBuffer* src, uint32_t j, uint32_t k) {
    if (i > buffer->size) i = buffer->size;
    if (j > src->size) j = src->size;
    if (k > src->size) k = src->size;
    if (j > k) j = k;

    int d = k - j;
    while (buffer->capacity < buffer->size + d)
        intbuffer_expand(buffer);

    for (int x = buffer->size - 1; x >= (int) i; x--) {
        buffer->data[x+d] = buffer->data[x];
    }

    for (int n = 0; n < d; n++) {
        buffer->data[i+n] = src->data[j+n];
    }

    buffer->size += d;
    buffer->damage = true;
}

void intbuffer_put_prefix (IntBuffer* buffer, uint32_t i, IntBuffer* src, uint32_t j) {
    intbuffer_put_substr(buffer, i, src, 0, j);
}

void intbuffer_put_suffix (IntBuffer* buffer, uint32_t i, IntBuffer* src, uint32_t j) {
    intbuffer_put_substr(buffer, i, src, j, src->size);
}


void intbuffer_get_substr (IntBuffer* buffer, uint32_t i, uint32_t j, CharBuffer* dst) {
    if (i > buffer->size) i = buffer->size;
    if (j > buffer->size) j = buffer->size;
    if (i > j) i = j;

    for (int n = i; n < j; n++) {
        char buf[5] = {0};

        int r = codepoint_to_chars(buf, buffer->data[n]);

        if (r == 0) {
            continue;
        }

        charbuffer_astr(dst, buf);
    }
}

void intbuffer_get_prefix (IntBuffer* buffer, uint32_t i, CharBuffer* dst) {
    intbuffer_get_substr(buffer, 0, i, dst);
}

void intbuffer_get_suffix (IntBuffer* buffer, uint32_t i, CharBuffer* dst) {
    intbuffer_get_substr(buffer, i, buffer->size, dst);
}


void intbuffer_rm_char (IntBuffer* buffer, uint32_t i) {
    intbuffer_rm_substr(buffer, i, i+1);
}

void intbuffer_rm_substr (IntBuffer* buffer, uint32_t i , uint32_t j) {
    if (i > buffer->size) i = buffer->size;
    if (j > buffer->size) j = buffer->size;
    if (i > j) i = j;

    int d = j - i;

    for (int n = i; n < buffer->size - d; n++) {
        buffer->data[n] = buffer->data[n+d];
    }

    buffer->size -= d;
    buffer->damage = true;
}

void intbuffer_rm_prefix (IntBuffer* buffer, uint32_t i) {
    intbuffer_rm_substr(buffer, 0, i);
}

void intbuffer_rm_suffix (IntBuffer* buffer, uint32_t i) {
    intbuffer_rm_substr(buffer, i, buffer->size);
}
