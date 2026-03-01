#pragma once

#include "main.h"

struct editor {

    Array* buffers;
    uint32_t current_buffer;

    CharBuffer* dir;

    Array* clipboard;

};


void editor_init (Editor* editor);

void editor_fini (Editor* editor);


bool editor_event (Editor* editor, InputEvent* event);

void editor_draw (Editor* editor, Box* window, MouseEvent* mev);
