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

    ['i'] = a_cursor_up,
    ['j'] = a_cursor_backward,
    ['k'] = a_cursor_down,
    ['l'] = a_cursor_forward,

    ['I'] = a_select_up,
    ['J'] = a_select_backward,
    ['K'] = a_select_down,
    ['L'] = a_select_backward,

    ['p'] = a_cursor_forward_word,
    ['o'] = a_cursor_backward_word,

    ['P'] = a_select_forward_word,
    ['O'] = a_select_backward_word,

    ['u'] = a_cursor_up_paragraph,
    ['h'] = a_cursor_down_paragraph,

    ['U'] = a_select_up_paragraph,
    ['H'] = a_select_down_paragraph,

    ['a'] = a_cursor_line_begin,
    ['z'] = a_cursor_line_end,

    ['A'] = a_select_line_begin,
    ['Z'] = a_select_line_end,

    ['w'] = a_select_word,
    ['e'] = a_select_line,

    ['s'] = a_selection_swap,
    ['S'] = a_selection_duplicate,

    ['d'] = a_delete,
    ['D'] = a_delete_lines,

    ['b'] = a_backspace,
    ['B'] = a_backspace_lines,

    [' '] = a_space,
    ['y'] = a_indent,
    ['Y'] = a_unindent,

    ['m'] = a_move_line_down,
    ['M'] = a_move_line_up,

    ['n'] = a_move_selection_forward,
    ['N'] = a_move_selection_backward,

    ['.'] = a_next_tab,
    [','] = a_prev_tab,

};

// Fixed Function Actions.
action_fn fixed_actions[MAX_FIXED_ACTIONS] = {
    [INPUT_TAB] = a_tab,
    [INPUT_SHIFT_TAB] = a_unindent,

    [INPUT_ENTER] = a_newline,
    [INPUT_BACKSPACE] = a_backspace,
    [INPUT_DELETE] = a_delete,

    [INPUT_UP] = a_cursor_up,
    [INPUT_DOWN] = a_cursor_down,
    [INPUT_RIGHT] = a_cursor_forward,
    [INPUT_LEFT] = a_cursor_backward,

    [INPUT_SHIFT_UP] = a_select_up,
    [INPUT_SHIFT_DOWN] = a_select_down,
    [INPUT_SHIFT_RIGHT] = a_select_forward,
    [INPUT_SHIFT_LEFT] = a_select_backward,

    [INPUT_ALT_UP] = a_cursor_up_paragraph,
    [INPUT_ALT_DOWN] = a_cursor_down_paragraph,
    [INPUT_ALT_LEFT] = a_cursor_line_begin,
    [INPUT_ALT_RIGHT] = a_cursor_line_end,

    [INPUT_SHIFT_ALT_UP] = a_select_up_paragraph,
    [INPUT_SHIFT_ALT_DOWN] = a_select_down_paragraph,
    [INPUT_SHIFT_ALT_LEFT] = a_select_line_begin,
    [INPUT_SHIFT_ALT_RIGHT] = a_select_line_end,

    [INPUT_CTRL_UP] = a_move_line_up,
    [INPUT_CTRL_DOWN] = a_move_line_down,


    [INPUT_CTRL_RIGHT] = a_cursor_forward_word,
    [INPUT_CTRL_LEFT] = a_cursor_backward_word,

    // [INPUT_SHIFT_CTRL_UP] = ,
    // [INPUT_SHIFT_CTRL_DOWN] = ,
    [INPUT_SHIFT_CTRL_RIGHT] = a_select_forward_word,
    [INPUT_SHIFT_CTRL_LEFT] = a_select_backward_word,

    [INPUT_HOME] = a_cursor_buffer_begin,
    [INPUT_END] = a_cursor_buffer_end,
};


// - Movement Actions - //

void a_cursor_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, -count, false);
}
void a_cursor_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, count, false);
}
void a_cursor_forward (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, count, false);
}
void a_cursor_backward (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, -count, false);
}

void a_cursor_forward_word (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, count, false);
}
void a_cursor_backward_word (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, -count, false);
}

void a_cursor_up_paragraph (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, -count, false);
}
void a_cursor_down_paragraph (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, count, false);
}

void a_cursor_line_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_begin(buffer, false);
}
void a_cursor_line_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_end(buffer, false);
}

void a_cursor_buffer_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_home(buffer, false);
}
void a_cursor_buffer_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_end(buffer, false);
}


// - Selection Actions - //

void a_select_forward (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, count, true);
}
void a_select_backward (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_char(buffer, -count, true);
}

void a_select_up (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, -count, true);
}
void a_select_down (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line(buffer, count, true);
}

void a_select_forward_word (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, count, true);
}
void a_select_backward_word (Editor* editor, Buffer* buffer, int32_t count) {
    int32_t lead = 1; // ...
    buffer_cursor_word(buffer, lead, -count, true);
}

void a_select_up_paragraph (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, -count, true);
}
void a_select_down_paragraph (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_paragraph(buffer, count, true);
}

void a_select_line_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_begin(buffer, true);
}
void a_select_line_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_line_end(buffer, true);
}

void a_select_buffer_begin (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_home(buffer, true);
}
void a_select_buffer_end (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_cursor_end(buffer, true);
}

void a_select_all (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_all(buffer);
}
void a_select_line (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_line(buffer);
}
void a_select_word (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_select_word(buffer);
}

void a_selection_swap (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_selection_swap(buffer);
}

void a_selection_duplicate (Editor* editor, Buffer* buffer, int32_t count) {
    buffer_selection_duplicate(buffer, count);
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
