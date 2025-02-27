#pragma once

#include "main.h"


// Action Function Template.
typedef void (*action_fn) (Editor* editor, Buffer* buffer, int32_t count);

// Action Function Arrays.
//  - Regular Actions.
//  - Map to Alt/Ctrl + Character.
extern action_fn actions[];
//
//  - Fixed-Function Actions.
//  - Map to Non-Character keys (like arrow keys).
extern action_fn fixed_actions[];

// Maximum Number of actions per action array.
enum {
    MAX_ACTIONS = 256,
    MAX_FIXED_ACTIONS = 128,
};


// template:
// void a_ (Editor* editor, Buffer* buffer, int32_t count);

// - Movement Actions - //

void a_up (Editor* editor, Buffer* buffer, int32_t count);
void a_down (Editor* editor, Buffer* buffer, int32_t count);
void a_left (Editor* editor, Buffer* buffer, int32_t count);
void a_right (Editor* editor, Buffer* buffer, int32_t count);

void a_word_left (Editor* editor, Buffer* buffer, int32_t count);
void a_word_right (Editor* editor, Buffer* buffer, int32_t count);

void a_line_begin (Editor* editor, Buffer* buffer, int32_t count);
void a_line_end (Editor* editor, Buffer* buffer, int32_t count);

void a_buffer_begin (Editor* editor, Buffer* buffer, int32_t count);
void a_buffer_end (Editor* editor, Buffer* buffer, int32_t count);

void a_paragraph_up (Editor* editor, Buffer* buffer, int32_t count);
void a_paragraph_down (Editor* editor, Buffer* buffer, int32_t count);

void a_to_matching (Editor* editor, Buffer* buffer, int32_t count);
void a_to_opening (Editor* editor, Buffer* buffer, int32_t count);
void a_to_closing (Editor* editor, Buffer* buffer, int32_t count);

void a_goto (Editor* editor, Buffer* buffer, int32_t count);

// - Selection Actions - //

void a_select_up (Editor* editor, Buffer* buffer, int32_t count);
void a_select_down (Editor* editor, Buffer* buffer, int32_t count);
void a_select_left (Editor* editor, Buffer* buffer, int32_t count);
void a_select_right (Editor* editor, Buffer* buffer, int32_t count);

void a_select_word (Editor* editor, Buffer* buffer, int32_t count);
void a_select_word_left (Editor* editor, Buffer* buffer, int32_t count);
void a_select_word_right (Editor* editor, Buffer* buffer, int32_t count);

void a_select_line (Editor* editor, Buffer* buffer, int32_t count);
void a_select_line_begin (Editor* editor, Buffer* buffer, int32_t count);
void a_select_line_end (Editor* editor, Buffer* buffer, int32_t count);

void a_select_all (Editor* editor, Buffer* buffer, int32_t count);
void a_select_buffer_begin (Editor* editor, Buffer* buffer, int32_t count);
void a_select_buffer_end (Editor* editor, Buffer* buffer, int32_t count);

void a_select_paragraph_up (Editor* editor, Buffer* buffer, int32_t count);
void a_select_paragraph_down (Editor* editor, Buffer* buffer, int32_t count);

void a_select_to_matching (Editor* editor, Buffer* buffer, int32_t count);
void a_select_to_opening (Editor* editor, Buffer* buffer, int32_t count);
void a_select_to_closing (Editor* editor, Buffer* buffer, int32_t count);

void a_select_swap (Editor* editor, Buffer* buffer, int32_t count);

// - Edit Actions - //

void a_tab (Editor* editor, Buffer* buffer, int32_t count);
void a_indent (Editor* editor, Buffer* buffer, int32_t count);
void a_unindent (Editor* editor, Buffer* buffer, int32_t count);

void a_space (Editor* editor, Buffer* buffer, int32_t count);
void a_newline (Editor* editor, Buffer* buffer, int32_t count);

void a_delete (Editor* editor, Buffer* buffer, int32_t count);
void a_backspace (Editor* editor, Buffer* buffer, int32_t count);
void a_delete_trailing_whitespace (Editor* editor, Buffer* buffer, int32_t count);

void a_duplicate (Editor* editor, Buffer* buffer, int32_t count);

void a_move_line_up (Editor* editor, Buffer* buffer, int32_t count);
void a_move_line_down (Editor* editor, Buffer* buffer, int32_t count);

void a_move_selection_forward (Editor* editor, Buffer* buffer, int32_t count);
void a_move_selection_backward (Editor* editor, Buffer* buffer, int32_t count);

// - Buffer Actions - //

void a_quit (Editor* editor, Buffer* buffer, int32_t count);
void a_close (Editor* editor, Buffer* buffer, int32_t count);

void a_new (Editor* editor, Buffer* buffer, int32_t count);
void a_open (Editor* editor, Buffer* buffer, int32_t count);
void a_save (Editor* editor, Buffer* buffer, int32_t count);
void a_save_all (Editor* editor, Buffer* buffer, int32_t count);
void a_save_as (Editor* editor, Buffer* buffer, int32_t count);

void a_next_tab (Editor* editor, Buffer* buffer, int32_t count);
void a_prev_tab (Editor* editor, Buffer* buffer, int32_t count);

// - Cut, Copy and Paste - //

void a_cut (Editor* editor, Buffer* buffer, int32_t count);
void a_copy (Editor* editor, Buffer* buffer, int32_t count);
void a_paste (Editor* editor, Buffer* buffer, int32_t count);

// - Undo / Redo - //

void a_undo (Editor* editor, Buffer* buffer, int32_t count);
void a_redo (Editor* editor, Buffer* buffer, int32_t count);

// - Find and Replace - //

void a_find (Editor* editor, Buffer* buffer, int32_t count);
void a_find_next (Editor* editor, Buffer* buffer, int32_t count);
void a_find_prev (Editor* editor, Buffer* buffer, int32_t count);
void a_replace (Editor* editor, Buffer* buffer, int32_t count);
