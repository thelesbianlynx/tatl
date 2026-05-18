#pragma once

#include "main.h"

struct editor {
    Array* buffers;
    uint32_t current_buffer;

    uint32_t altmode;
    TextBuffer* altbuffer;
    TextView* altview;

    CharBuffer* dir;

    Array* clipboard;

    int32_t tab_scroll;
    int32_t tab_scroll_dmg;

    Array* search_files;
    int32_t search_selection;
    int32_t search_scroll;
    bool search_scroll_dmg;
};


void editor_init (Editor* editor, Array* filename);

void editor_fini (Editor* editor);


bool editor_event (Editor* editor, InputEvent* event);

void editor_draw (Editor* editor, Box* window, MouseEvent* mev);
