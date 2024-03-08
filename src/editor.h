#pragma once

#include "main.h"
#include "input.h"

struct editor {
    uint32_t width, height;

    /* TEMP */
    uint32_t last_status, last_keycode;
};


void editor_init (struct editor* editor);
void editor_fini (struct editor* editor);

bool editor_update (struct editor* editor, uint32_t input_status, struct input_state* input_state);
void editor_draw (struct editor* editor, uint32_t width, uint32_t height);
