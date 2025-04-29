#include "actions.h"

#include "input.h"
#include "array.h"
#include "editor.h"
#include "buffer.h"
#include "charbuffer.h"

#define CTRL(CH) (CH - 'A' + 1)

// - Action Arrays - //

// Regular Actions.
action_fn actions[MAX_ACTIONS] = {

    // -- CTRL Actions -- //

    [CTRL('N')] = a_new,
    [CTRL('O')] = a_open,
    [CTRL('S')] = a_save_as,

    [CTRL('Q')] = a_quit,
    [CTRL('W')] = a_close,

    [CTRL('A')] = a_select_all,
    [CTRL('I')] = a_tab,
    [CTRL('M')] = a_newline,

    [CTRL('X')] = a_cut,
    [CTRL('C')] = a_copy,
    [CTRL('V')] = a_paste,

    [CTRL('F')] = a_find,
    [CTRL('G')] = a_find_next,
    [CTRL('H')] = a_find_prev,
    [CTRL('R')] = a_replace,

    [CTRL('Z')] = a_undo,
    [CTRL('Y')] = a_redo,


    // -- Alt Actions -- //

    ['i'] = a_up,
    ['j'] = a_left,
    ['k'] = a_down,
    ['l'] = a_right,

    ['I'] = a_select_up,
    ['J'] = a_select_left,
    ['K'] = a_select_down,
    ['L'] = a_select_right,

    ['o'] = a_word_left,
    ['p'] = a_word_right,

    ['O'] = a_select_word_left,
    ['P'] = a_select_word_right,

    ['a'] = a_line_begin,
    ['z'] = a_line_end,

    ['A'] = a_select_line_begin,
    ['Z'] = a_select_line_end,

    [','] = a_paragraph_up,
    ['.'] = a_paragraph_down,

    ['<'] = a_select_paragraph_up,
    ['>'] = a_select_paragraph_down,

    ['\\'] = a_to_matching,
    ['['] = a_to_opening,
    [']'] = a_to_closing,

    ['|'] = a_select_to_matching,
    ['{'] = a_select_to_opening,
    ['}'] = a_select_to_closing,

    ['w'] = a_select_word,
    ['e'] = a_select_line,

    ['s'] = a_select_swap,
    ['S'] = a_duplicate,

    ['d'] = a_delete,
    ['D'] = a_delete_lines,

    ['b'] = a_backspace,
    ['B'] = a_backspace_lines,

    [' '] = a_space,
    ['y'] = a_indent,
    ['Y'] = a_unindent,

    ['m'] = a_move_line_down,
    ['M'] = a_move_line_up,


    ['t'] = a_next_tab,
    ['T'] = a_prev_tab,


};

// Fixed Function Actions.
action_fn fixed_actions[MAX_FIXED_ACTIONS] = {
    [INPUT_TAB] = a_tab,
    [INPUT_SHIFT_TAB] = a_unindent,

    [INPUT_ENTER] = a_newline,
    [INPUT_BACKSPACE] = a_backspace,
    [INPUT_DELETE] = a_delete,

    [INPUT_UP] = a_up,
    [INPUT_DOWN] = a_down,
    [INPUT_LEFT] = a_left,
    [INPUT_RIGHT] = a_right,

    [INPUT_SHIFT_UP] = a_select_up,
    [INPUT_SHIFT_DOWN] = a_select_down,
    [INPUT_SHIFT_LEFT] = a_select_left,
    [INPUT_SHIFT_RIGHT] = a_select_right,

    [INPUT_ALT_UP] = a_paragraph_up,
    [INPUT_ALT_DOWN] = a_paragraph_down,
    [INPUT_ALT_LEFT] = a_line_begin,
    [INPUT_ALT_RIGHT] = a_line_end,

    [INPUT_SHIFT_ALT_UP] = a_select_paragraph_up,
    [INPUT_SHIFT_ALT_DOWN] = a_select_paragraph_down,
    [INPUT_SHIFT_ALT_LEFT] = a_select_line_begin,
    [INPUT_SHIFT_ALT_RIGHT] = a_select_line_end,

    [INPUT_CTRL_UP] = a_move_line_up,
    [INPUT_CTRL_DOWN] = a_move_line_down,
    [INPUT_CTRL_LEFT] = a_word_left,
    [INPUT_CTRL_RIGHT] = a_word_right,

    // [INPUT_SHIFT_CTRL_UP] = ,
    // [INPUT_SHIFT_CTRL_DOWN] = ,
    [INPUT_SHIFT_CTRL_LEFT] = a_select_word_left,
    [INPUT_SHIFT_CTRL_RIGHT] = a_select_word_right,

    [INPUT_HOME] = a_buffer_begin,
    [INPUT_END] = a_select_buffer_end,
};


// - Movement Actions - //

void a_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, -count, false);
}
void a_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, count, false);
}
void a_left (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, -count, false);
}
void a_right (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, count, false);
}

void a_word_left (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, -count, false);
}
void a_word_right (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, count, false);
}

void a_line_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_begin(buffer, false);
}
void a_line_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_end(buffer, false);
}

void a_buffer_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_home(buffer, false);
}
void a_buffer_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_end(buffer, false);
}

void a_paragraph_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, -count, false);
}
void a_paragraph_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, count, false);
}

void a_to_matching (Editor* editor, Buffer* buffer, int32_t count) {}
void a_to_opening (Editor* editor, Buffer* buffer, int32_t count) {}
void a_to_closing (Editor* editor, Buffer* buffer, int32_t count) {}

void a_goto (Editor* editor, Buffer* buffer, int32_t count) {}

// - Selection Actions - //

void a_select_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, -count, true);
}
void a_select_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, count, true);
}
void a_select_left (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, -count, true);
}
void a_select_right (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, count, true);
}

void a_select_word (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_word(buffer);
}
void a_select_word_left (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, -count, true);
}
void a_select_word_right (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, count, true);
}

void a_select_line (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_line(buffer);
}
void a_select_line_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_begin(buffer, true);
}
void a_select_line_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_end(buffer, true);
}

void a_select_all (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_all(buffer);
}
void a_select_buffer_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_home(buffer, true);
}
void a_select_buffer_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_end(buffer, true);
}

void a_select_paragraph_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, -count, true);
}
void a_select_paragraph_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, count, true);
}

void a_select_to_matching (Editor* editor, Buffer* buffer, int32_t count) {}
void a_select_to_opening (Editor* editor, Buffer* buffer, int32_t count) {}
void a_select_to_closing (Editor* editor, Buffer* buffer, int32_t count) {}

void a_select_swap (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_selection_swap(buffer);
}

// - Edit Actions - //

void a_tab (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_tab(buffer, count);
}
void a_indent (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_indent(buffer, count);
}
void a_unindent (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_indent(buffer, -count);
}

void a_space (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_char(buffer, ' ', count);
}
void a_newline (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_line(buffer, count);
}

void a_delete (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_delete(buffer, count);
}
void a_backspace (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_backspace(buffer, count);
}
void a_delete_lines (Editor* editor, Buffer* buffer, int32_t count) {

}
void a_backspace_lines (Editor* editor, Buffer* buffer, int32_t count) {

}

void a_delete_trailing_whitespace (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_selection_delete_whitespace(buffer);
}

void a_duplicate (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_selection_duplicate(buffer, count);
}

void a_move_line_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_move_line(buffer, -count);
}
void a_move_line_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_edit_move_line(buffer, count);
}

void a_move_selection_forward (Editor* editor, Buffer* buffer, int32_t count) {

}
void a_move_selection_backward (Editor* editor, Buffer* buffer, int32_t count) {

}

// - Buffer Actions - //

void a_quit (Editor* editor, Buffer* buffer, int32_t count) {
    editor_quit(editor);
}
void a_close (Editor* editor, Buffer* buffer, int32_t count) {
    editor_close(editor);
}

void a_new (Editor* editor, Buffer* buffer, int32_t count) {
    editor_new(editor);
}
void a_open (Editor* editor, Buffer* buffer, int32_t count) {
    editor_open(editor);
}
void a_save (Editor* editor, Buffer* buffer, int32_t count) {
    editor_save(editor);
}
void a_save_all (Editor* editor, Buffer* buffer, int32_t count) {
    editor_save_all(editor);
}
void a_save_as (Editor* editor, Buffer* buffer, int32_t count) {
    editor_save_as(editor);
}

void a_next_tab (Editor* editor, Buffer* buffer, int32_t count) {
    editor_buffer_next(editor);
}
void a_prev_tab (Editor* editor, Buffer* buffer, int32_t count) {
    editor_buffer_prev(editor);
}

// - Cut, Copy and Paste - //

void a_cut (Editor* editor, Buffer* buffer, int32_t count) {
    CharBuffer* clipboard = editor_get_clipboard(editor);
    charbuffer_clear(clipboard);
    buffer_selection_cut(buffer, clipboard);
}

void a_copy (Editor* editor, Buffer* buffer, int32_t count) {
    CharBuffer* clipboard = editor_get_clipboard(editor);
    charbuffer_clear(clipboard);
    buffer_selection_copy(buffer, clipboard);
}

void a_paste (Editor* editor, Buffer* buffer, int32_t count) {
    CharBuffer* clipboard = editor_get_clipboard(editor);
    buffer_edit_text(buffer, clipboard, count);
}

// - Undo / Redo - //

void a_undo (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_undo(buffer, count);
}
void a_redo (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_redo(buffer, count);
}

// - Find and Replace - //


void a_find (Editor* editor, Buffer* buffer, int32_t count) {}
void a_find_next (Editor* editor, Buffer* buffer, int32_t count) {}
void a_find_prev (Editor* editor, Buffer* buffer, int32_t count) {}
void a_replace (Editor* editor, Buffer* buffer, int32_t count) {}
