#pragma once

#include "main.h"


struct editor {
    Array* buffers;
    int32_t buffer_id;

    CharBuffer* clipboard;

    bool edit_mode;

    uint32_t mstate;
    uint32_t mx;
    uint32_t my;

    int32_t debug;
};


void editor_init (Editor* editor);
void editor_fini (Editor* editor);

bool editor_update (Editor* editor, InputStatus status, InputState* state);
void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug);

CharBuffer* editor_get_clipboard (Editor* editor);

void editor_edit_mode (Editor* editor);

void editor_escape (Editor* editor);
bool editor_confirm (Editor* editor);
bool editor_discard (Editor* editor);

void editor_save (Editor* editor);
void editor_saveall (Editor* editor);

void editor_new (Editor* editor);
void editor_open (Editor* editor);
void editor_duplicate (Editor* editor);

void editor_buffer_next (Editor* editor);
void editor_buffer_prev (Editor* editor);

void editor_close (Editor* editor);
void editor_quit (Editor* editor);
