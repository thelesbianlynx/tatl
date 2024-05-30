#pragma once

#include "main.h"

#include "output.h"


struct point {
    int32_t line, col;

};

struct buffer {
    Array* lines;

    Point cursor;
    Point selection;

    int32_t view_line;

    char* filename;

    int32_t tab_width;
    bool hard_tabs;

    // Undo/Redo Stack.
    // ...

    // Buffer-Specific Clipboard(s).
    // ...
};


Buffer* buffer_create (char* file);

void buffer_destroy (Buffer* buffer);

void buffer_save (Buffer* buffer);

void buffer_draw (Buffer* buffer, Box window);


void buffer_edit_char (Buffer* buffer, uint32_t ch, int32_t i);

void buffer_edit_text (Buffer* buffer, CharBuffer* text, int32_t i);

void buffer_edit_line (Buffer* buffer, int32_t i);

void buffer_edit_tab (Buffer* buffer, int32_t i);

void buffer_edit_indent (Buffer* buffer, int32_t i);

void buffer_edit_delete (Buffer* buffer, int32_t i);

void buffer_edit_backspace (Buffer* buffer, int32_t i);


void buffer_cursor_goto (Buffer* buffer, int32_t row, int32_t col, bool sel);

void buffer_cursor_char (Buffer* buffer, int32_t i, bool sel);

void buffer_cursor_line (Buffer* buffer, int32_t i, bool sel);

void buffer_cursor_word (Buffer* buffer, int32_t i, bool sel);

void buffer_cursor_home (Buffer* buffer, bool sel);

void buffer_cursor_end (Buffer* buffer, bool sel);

void buffer_cursor_line_begin (Buffer* buffer, bool sel);

void buffer_cursor_line_end (Buffer* buffer, bool sel);


void buffer_undo (Buffer* buffer, int32_t i);

void buffer_redo (Buffer* buffer, int32_t i);


uint32_t buffer_selection_get_length (Buffer* buffer);

char* buffer_selection_get_text (Buffer* buffer);


char* buffer_clipboard_get_text (Buffer* buffer, uint32_t i);

void buffer_clipboard_set_text (Buffer* buffer, uint32_t i, const char* text);
