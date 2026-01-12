#pragma once

#include "main.h"

#ifdef TATL_DEPRECATED


struct editor {
    Array* buffers;
    int32_t buffer_id;

    CharBuffer* tab_bar;
    int32_t tab_scroll;
    bool tab_scroll_dmg;

    CharBuffer* clipboard;

    uint32_t alt_mode;
    Buffer* alt_buffer;
    Buffer* target_buffer;

    bool should_quit;

    uint32_t mstate;
    uint32_t mx;
    uint32_t my;

};


void editor_init (Editor* editor, Array* filenames);
void editor_fini (Editor* editor);

bool editor_update (Editor* editor, InputEvent* event);
void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug);

CharBuffer* editor_get_clipboard (Editor* editor);

Buffer* editor_new (Editor* editor);
void editor_open (Editor* editor);
void editor_save (Editor* editor);
void editor_save_all (Editor* editor);
void editor_save_as (Editor* editor);

void editor_close (Editor* editor);
void editor_quit (Editor* editor);
void editor_buffer_next (Editor* editor);
void editor_buffer_prev (Editor* editor);

void editor_altbuffer_enter (Editor* editor);
void editor_altbuffer_tab (Editor* editor);
void editor_altbuffer_up (Editor* editor);
void editor_altbuffer_down (Editor* editor);

#endif
