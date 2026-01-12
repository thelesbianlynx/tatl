#pragma once

#include "main.h"

struct selection {
    int32_t cursor, anchor;
    int32_t col_mem;

    bool primary;
};

struct textbuffer {
    Rope* text;

    Array* undo;
    Array* redo;

    bool action_state;
    uint32_t action_type;

    Array* selections;
    Array* pre_selections;

    uint32_t tab_width;
    bool hard_tabs;

    bool altmode;

    bool cursor_dmg;
    bool text_dmg;
};


TextBuffer* textbuffer_create (Rope* text);

void textbuffer_destroy (TextBuffer* buffer);


void textbuffer_undo (TextBuffer* buffer);

void textbuffer_redo (TextBuffer* buffer);



void textbuffer_edit_char (TextBuffer* buffer, uint32_t ch, int32_t i);

void textbuffer_edit_text (TextBuffer* buffer, Rope* text, int32_t i);


void textbuffer_edit_newline (TextBuffer* buffer, int32_t i);

void textbuffer_edit_tab (TextBuffer* buffer, int32_t i);

void textbuffer_edit_indent (TextBuffer* buffer, int32_t i);


void textbuffer_edit_delete (TextBuffer* buffer, int32_t i);

void textbuffer_edit_delete_lines (TextBuffer* buffer, int32_t i);

void textbuffer_edit_backspace (TextBuffer* buffer, int32_t i);

void textbuffer_edit_backspace_lines (TextBuffer* buffer, int32_t i);


void textbuffer_edit_move_lines (TextBuffer* buffer, int32_t i);



void textbuffer_cursor_char (TextBuffer* buffer, int32_t i, bool s);


void textbuffer_cursor_line (TextBuffer* buffer, int32_t i, bool s);

void textbuffer_cursor_line_begin (TextBuffer* buffer, int32_t i, bool s);

void textbuffer_cursor_line_end (TextBuffer* buffer, int32_t i, bool s);

void textbuffer_cursor_paragraph (TextBuffer* buffer, int32_t i, bool s);


void textbuffer_cursor_word_begin (TextBuffer* buffer, int32_t i, bool s);

void textbuffer_cursor_word_end (TextBuffer* buffer, int32_t i, bool s);


void textbuffer_cursor_goto (TextBuffer* buffer, int32_t row, int32_t col, bool s);



void textbuffer_select_word (TextBuffer* buffer, int32_t i, bool a);

void textbuffer_select_line (TextBuffer* buffer, int32_t i, bool a);


void textbuffer_select_parens (TextBuffer* buffer, int32_t i, bool a);

void textbuffer_select_brackets (TextBuffer* buffer, int32_t i, bool a);

void textbuffer_select_braces (TextBuffer* buffer, int32_t i, bool a);

void textbuffer_select_quotes (TextBuffer* buffer, int32_t i, bool a);

void textbuffer_select_squotes (TextBuffer* buffer, int32_t i, bool a);



void textbuffer_selection_clear (TextBuffer* buffer);

void textbuffer_selection_swap (TextBuffer* buffer);


void textbuffer_selection_next (TextBuffer* buffer, int32_t i);

void textbuffer_selection_next_word (TextBuffer* buffer, int32_t i);

void textbuffer_selection_next_line (TextBuffer* buffer, int32_t i);


void textbuffer_selection_add_next (TextBuffer* buffer, int32_t i);

void textbuffer_selection_add_next_word (TextBuffer* buffer, int32_t i);

void textbuffer_selection_add_next_line (TextBuffer* buffer, int32_t i);
