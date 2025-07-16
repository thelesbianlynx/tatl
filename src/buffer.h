#pragma once

#include "main.h"


struct point {
    int32_t line, col;
};

struct buffer {
    Editor* editor;

    Array* lines;

    Point cursor;
    Point selection;

    int32_t col_mem;

    bool scroll_damage;
    int32_t scroll_line;
    int32_t scroll_offset;
    int32_t scroll_pages;

    CharBuffer* title;
    CharBuffer* filename;

    bool modified;

    bool alt_mode;
    bool page_mode;
    bool block_mode;
    bool raise_mode;

    int32_t tab_width;
    bool hard_tabs;

    bool should_close;

    // Undo/Redo Stack.
    UStack* ustack;
    Point pre_cursor;
    uint32_t pre_commit_type;
    uint32_t commit_level;

    // Buffer-Specific Clipboard(s).
    // ...
};


Buffer* buffer_create (Editor* editor, const char* title);

void buffer_destroy (Buffer* buffer);

void buffer_load (Buffer* buffer, const char* filename);

bool buffer_save (Buffer* buffer);

void buffer_save_as (Buffer* buffer, const char* filename);

void buffer_title (Buffer* buffer, const char* title);

void buffer_prompt (Buffer* buffer, const char* prompt);

bool buffer_empty (Buffer* buffer);


void buffer_draw (Buffer* buffer, Box window, uint32_t mstate, uint32_t mx, uint32_t my);


void buffer_edit_char (Buffer* buffer, uint32_t ch, int32_t i);

void buffer_edit_text (Buffer* buffer, CharBuffer* text, int32_t i);

void buffer_edit_line (Buffer* buffer, int32_t i);

void buffer_edit_tab (Buffer* buffer, int32_t i);

void buffer_edit_indent (Buffer* buffer, int32_t i);

void buffer_edit_delete (Buffer* buffer, int32_t i);

void buffer_edit_backspace (Buffer* buffer, int32_t i);

void buffer_edit_delete_lines (Buffer* buffer, int32_t i);

void buffer_edit_backspace_lines (Buffer* buffer, int32_t i);

void buffer_edit_move_line (Buffer* buffer, int32_t i);

void buffer_edit_move_selection (Buffer* buffer, int32_t i);


void buffer_cursor_goto (Buffer* buffer, int32_t row, int32_t col, bool sel);

void buffer_cursor_line (Buffer* buffer, int32_t i, bool sel);

void buffer_cursor_char (Buffer* buffer, int32_t i, bool sel);

void buffer_cursor_word (Buffer* buffer, int32_t lead, int32_t i, bool sel);

void buffer_cursor_paragraph (Buffer* buffer, int32_t lead, int32_t i, bool sel);

void buffer_cursor_home (Buffer* buffer, bool sel);

void buffer_cursor_end (Buffer* buffer, bool sel);

void buffer_cursor_line_begin (Buffer* buffer, bool sel);

void buffer_cursor_line_end (Buffer* buffer, bool sel);


void buffer_select_all (Buffer* buffer);

void buffer_select_line (Buffer* buffer);

void buffer_select_word (Buffer* buffer);


bool buffer_selection_exist (Buffer* buffer);

void buffer_selection_swap (Buffer* buffer);

void buffer_selection_clear (Buffer* buffer);

void buffer_selection_cut (Buffer* buffer, CharBuffer* dst);

void buffer_selection_copy (Buffer* buffer, CharBuffer* dst);

void buffer_selection_duplicate (Buffer* buffer, int32_t i);

void buffer_selection_delete_whitespace (Buffer* buffer);


void buffer_get_contents (Buffer* buffer, CharBuffer* dst);


void buffer_undo (Buffer* buffer, int32_t i);

void buffer_redo (Buffer* buffer, int32_t i);
