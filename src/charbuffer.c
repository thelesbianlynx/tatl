#include "charbuffer.h"


// Create Char Buffer.
CharBuffer* charbuffer_create () {
    CharBuffer* cb = malloc (sizeof(CharBuffer));
    cb->size = 0;
    cb->capacity = 1024;
    cb->buffer = calloc(cb->capacity, sizeof(char));
    cb->damage = true;
    cb->lineno = 0;
    return cb;
}

// Destroy Char Builder.
//  - If the raw buffer data is needed longer than the wrapper object, then set field 'buffer' to NULL
//      before calling destroy. (After saving the buffer somewhere else obviously.)
void charbuffer_destroy (CharBuffer* cb) {
    free(cb->buffer);
    free(cb);
}

// Clear buffer
//  - Set size to 0.
void charbuffer_clear (CharBuffer* cb) {
    cb->size = 0;
}

// Expand Char Buffer.
//
static void charbuffer_expand (CharBuffer* cb, uint32_t new_capacity) {
    char* new_buffer = calloc(new_capacity, sizeof(char));

    for (int i = 0; i < cb->size; ++i)
        new_buffer[i] = cb->buffer[i];

    free(cb->buffer);
    cb->buffer = new_buffer;
    cb->capacity = new_capacity;
}

// Append Character.
void charbuffer_achar (CharBuffer* cb, char ch) {
    if (cb->capacity <= cb->size + 2)
        charbuffer_expand(cb, cb->capacity*2);

    cb->buffer[cb->size] = ch;
    cb->size++;
    cb->damage = true;
}

// Append Characters.
void charbuffer_achars (CharBuffer* cb, CharBuffer* src) {
    uint32_t cap = cb->capacity;
    while (cap <= cb->size + src->size + 1)
        cap *= 2;

    if (cap > cb->capacity)
        charbuffer_expand(cb, cap);

    for (int i = 0; i < src->size; ++i)
        cb->buffer[cb->size + i] = src->buffer[i];

    cb->size += src->size;
    cb->damage = true;
}

// Append String.
void charbuffer_astr (CharBuffer* cb, const char* str) {
    uint32_t len = strlen(str);
    uint32_t cap = cb->capacity;
    while (cap <= cb->size + len + 1)
        cap *= 2;

    if (cap > cb->capacity)
        charbuffer_expand(cb, cap);

    for (int i = 0; i < len; ++i)
        cb->buffer[cb->size + i] = str[i];

    cb->size += len;
    cb->damage = true;
}

// Insert Character.
void charbuffer_ichar (CharBuffer* cb, char ch, uint32_t i) {
    if (i >= cb->size) {
        charbuffer_achar(cb, ch);
        return;
    }

    if (cb->capacity <= cb->size + 2)
        charbuffer_expand(cb, cb->capacity*2);

    for (int x = cb->size - 1; x >= (int) i; --x)
        cb->buffer[x + 1] = cb->buffer[x];

    cb->buffer[i] = ch;
    cb->size++;
    cb->damage = true;
}

// Insert Characters.
void charbuffer_ichars (CharBuffer* cb, CharBuffer* src, uint32_t i) {
    if (i >= cb->size) {
        charbuffer_achars(cb, src);
        return;
    }

    uint32_t cap = cb->capacity;
    while (cap <= cb->size + src->size + 1)
        cap *= 2;

    if (cap > cb->capacity)
        charbuffer_expand(cb, cap);

    for (int x = cb->size - 1; x >= (int) i; --x)
        cb->buffer[x + src->size] = cb->buffer[x];

    for (int x = 0; x < src->size; ++x)
        cb->buffer[x + i] = src->buffer[x];

    cb->size += src->size;
    cb->damage = true;
}

// Insert String.
void charbuffer_istr (CharBuffer* cb, const char* str, uint32_t i){
    if (i >= cb->size) {
        charbuffer_astr(cb, str);
        return;
    }

    uint32_t len = strlen(str);
    uint32_t cap = cb->capacity;
    while (cap <= cb->size + len + 1)
        cap *= 2;

    if (cap > cb->capacity)
        charbuffer_expand(cb, cap);

    for (int x = cb->size - 1; x >= (int) i; --x)
        cb->buffer[x + len] = cb->buffer[x];

    for (int x = 0; x < len; ++x)
        cb->buffer[x + i] = str[x];

    cb->size += len;
    cb->damage = true;
}

uint32_t charbuffer_get (CharBuffer* cb, uint32_t i) {
    if (i >= cb->size) return 0;
    else return cb->buffer[i];
}

void charbuffer_get_substr (CharBuffer* cb, CharBuffer* dst, uint32_t i, uint32_t j) {
    if (i >= j) return;
    if (i >= cb->size) return;
    if (j > cb->size) j = cb->size;

    int len = j - i;
    int cap = dst->capacity;
    while (cap <= dst->size + len + 1)
    cap *= 2;

    if (cap > dst->capacity)
        charbuffer_expand(dst, cap);

    for (int x = 0; x < len; ++x) {
        dst->buffer[dst->size + x] = cb->buffer[i+x];
    }

    dst->size += len;
    dst->damage = true;
}

void charbuffer_get_prefix (CharBuffer* cb, CharBuffer* dst, uint32_t i) {
    charbuffer_get_substr(cb, dst, 0, i);
}

void charbuffer_get_suffix (CharBuffer* cb, CharBuffer* dst, uint32_t i) {
    charbuffer_get_substr(cb, dst, i, cb->size);
}

void charbuffer_copy (CharBuffer* cb, CharBuffer* dst) {
    charbuffer_get_substr(cb, dst, 0, cb->size);
}


void charbuffer_rm_char (CharBuffer* cb, uint32_t i) {
    charbuffer_rm_substr(cb, i, i+1);
}

void charbuffer_rm_substr (CharBuffer* cb, uint32_t i, uint32_t j) {
    if (i >= j) return;
    if (i >= cb->size) return;
    if (j > cb->size) j = cb->size;

    uint32_t len = j - i;

    for (int x = i; x < cb->size - len + 1; ++x) {
        cb->buffer[x] = cb->buffer[x+len];
    }

    cb->size -= len;
    cb->damage = true;
}

void charbuffer_rm_prefix (CharBuffer* cb, uint32_t i) {
    charbuffer_rm_substr(cb, 0, i);
}

void charbuffer_rm_suffix (CharBuffer* cb, uint32_t i) {
    charbuffer_rm_substr(cb, i, cb->size);
}



// Read contents of file into buffer
void charbuffer_read (CharBuffer* cb, FILE* file) {
    int c;

    while ((c = getc(file)) != EOF) {
        charbuffer_achar(cb, (char) c);
    }
}

// Read contents of file into buffer, one line at a time.
bool charbuffer_read_line (CharBuffer* cb, FILE* file) {
    int c;

    while ((c = getc(file)) != EOF) {
        if (c == '\n') return true;

        charbuffer_achar(cb, (char) c);
    }

    return false;
}


void charbuffer_write (CharBuffer* cb, FILE* file) {

}
