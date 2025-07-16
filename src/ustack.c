#include "ustack.h"

#include "array.h"
#include "charbuffer.h"
#include "buffer.h"

typedef struct line Line;
typedef struct generation Generation;

struct line {
    CharBuffer* line;
    int32_t rc;
};

struct generation {
    Array* lines;
    Point pre_cursor;
    Point post_cursor;
};


static
Line* line_create (CharBuffer* buffer) {
    Line* line = malloc(sizeof(Line));
    line->line = charbuffer_create();
    line->rc = 1;

    charbuffer_achars(line->line, buffer);

    return line;
}

static
void line_ref (Line* line) {
    line->rc++;
}

static
void line_unref (Line* line) {
    line->rc--;
    if (line->rc <= 0) {
        charbuffer_destroy(line->line);
        free(line);
    }
}


static
Generation* generation_create (Array* lines, Point pre_cursor, Point post_cursor) {
    Generation* gen = malloc(sizeof(Generation));
    gen->lines = lines;
    gen->pre_cursor = pre_cursor;
    gen->post_cursor = post_cursor;
    return gen;
}

static
void generation_destroy (Generation* gen) {
    for (int i = 0; i < gen->lines->size; i++) {
        line_unref(gen->lines->data[i]);
    }
    array_destroy(gen->lines);
    free(gen);
}

static
void clear_gen_list (Array* genlist) {
    for (int i = 0; i < genlist->size; i++) {
        generation_destroy(genlist->data[i]);
    }
    array_clear(genlist);
}

static
Generation* get_generation (Array* genlist) {
    if (genlist->size == 0) return NULL;
    return genlist->data[genlist->size - 1];
}

static
Generation* pop_generation (Array* genlist) {
    if (genlist->size == 0) return NULL;
    return array_remove(genlist, genlist->size - 1);
}


UStack* ustack_create () {
    UStack* ustack = malloc(sizeof(UStack));
    ustack->undo = array_create();
    ustack->redo = array_create();
    return ustack;
}

void ustack_destroy (UStack* ustack) {
    clear_gen_list(ustack->undo);
    clear_gen_list(ustack->redo);
    array_destroy(ustack->undo);
    array_destroy(ustack->redo);
    free(ustack);
}

void ustack_clear (UStack* ustack) {
    clear_gen_list(ustack->undo);
    clear_gen_list(ustack->redo);
}

void ustack_push (UStack* ustack, Array* lines, Point* pre_cursor, Point* post_cursor) {
    clear_gen_list(ustack->redo);

    Array* newgen_array = array_create_capacity(lines->size + 1);

    Generation* lastgen = get_generation(ustack->undo);

    if (lastgen == NULL) {
        // First Push.
        for (int i = 0; i < lines->size; i++) {
            array_add(newgen_array, line_create(lines->data[i]));
        }
    } else {
        for (int i = 0; i < lines->size; i++) {
            CharBuffer* line = lines->data[i];
            if (line->damage) {
                array_add(newgen_array, line_create(line));
                line->damage = false;
            } else {
                Line* old_line = lastgen->lines->data[line->lineno];
                line_ref(old_line);
                array_add(newgen_array, old_line);
            }
            line->lineno = i;
        }
    }

    Generation* newgen = generation_create(newgen_array, *pre_cursor, *post_cursor);
    array_add(ustack->undo, newgen);
}

static
void restore (Generation* gen, Array* lines) {
    // Clear Lines.
    for (int i = 0; i < lines->size; i++) {
        charbuffer_destroy(lines->data[i]);
    }
    array_clear(lines);

    // Restore Lines.
    for (int i = 0; i < gen->lines->size; i++) {
        Line* line = gen->lines->data[i];
        CharBuffer* cb = charbuffer_create();
        charbuffer_achars(cb, line->line);
        array_add(lines, cb);
    }
}

void ustack_undo (UStack* ustack, Array* lines, Point* cursor) {
    if (ustack->undo->size < 2) return;

    // Pop _CURRENT_ generation from the undo stack and push to redo stack.
    Generation* current = pop_generation(ustack->undo);
    array_add(ustack->redo, current);

    // Restore previous generation (now current).
    Generation* last = get_generation(ustack->undo);
    restore(last, lines);
    *cursor = last->pre_cursor;
}

void ustack_redo (UStack* ustack, Array* lines, Point* cursor) {
    // Pop Generation from redo stack, restore and push to undo stack.
    Generation* gen = pop_generation(ustack->redo);
    if (gen == NULL) return;

    array_add(ustack->undo, gen);
    restore(gen, lines);
    *cursor = gen->post_cursor;
}
