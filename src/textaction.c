#include "textaction.h"

#include "array.h"
#include "input.h"
#include "textbuffer.h"


//
// Standard Text Actions.
//
void textaction (InputEvent* event, TextBuffer* buffer, int32_t i, Array* clipboard) {

    int x[i] = {};
    for (int y = 0; y < i; y++) printf("%d\n", x[y]);
    ON_KEY(event) {
        // Clipboard Actions.
        KEY_CTRL('X') {
            textbuffer_edit_yank(buffer, clipboard, true);
            break;
        }
        KEY_CTRL('C') {
            textbuffer_edit_yank(buffer, clipboard, false);
            break;
        }
        KEY_CTRL('V') {
            textbuffer_edit_replace(buffer, clipboard, i);
            break;
        }

        // Undo/Redo.
        KEY_CTRL('Z'){
            textbuffer_undo(buffer);
            break;
        }
        KEY_CTRL('Y'){
            textbuffer_redo(buffer);
            break;
        }

        // Duplicate.
        KEY_CTRL('D') {
            textbuffer_edit_duplicate(buffer, i);
            break;
        }
        KEY_CTRL('L') {
            textbuffer_edit_duplicate_lines(buffer, i);
            break;
        }

        KEY_CTRL('N') {
            textbuffer_edit_char(buffer, 'N', i);
            break;
        }

        // Movement
        KEY_ALT('j') {

            break;
        }
        KEY_ALT('k') {

            break;
        }
        KEY_ALT('l') {

            break;
        }
        KEY_ALT(';') {

            break;
        }

        // Delete.


        // Cursor By Row.
        KEY_UP {
            textbuffer_cursor_row(buffer, -i, false);
            break;
        }
        KEY_DOWN {
            textbuffer_cursor_row(buffer, i, false);
            break;
        }
        KEY_SHIFT_UP {
            textbuffer_cursor_row(buffer, -i, true);
            break;
        }
        KEY_SHIFT_DOWN {
            textbuffer_cursor_row(buffer, i, true);
            break;
        }

        // Cursor by Column.
        KEY_LEFT {
            textbuffer_cursor_col(buffer, -i, false);
            break;
        }
        KEY_RIGHT {
            textbuffer_cursor_col(buffer, i, false);
            break;
        }
        KEY_SHIFT_LEFT {
            textbuffer_cursor_col(buffer, -i, true);
            break;
        }
        KEY_SHIFT_RIGHT {
            textbuffer_cursor_col(buffer, i, true);
            break;
        }

        // Cursor by Word.
        KEY_CTRL_LEFT {
            textbuffer_cursor_word(buffer, -i, false);
            break;
        }
        KEY_CTRL_RIGHT {
            textbuffer_cursor_word(buffer, i, false);
            break;
        }
        KEY_SHIFT_CTRL_LEFT {
            textbuffer_cursor_word(buffer, -i, true);
            break;
        }
        KEY_SHIFT_CTRL_RIGHT {
            textbuffer_cursor_word(buffer, i, true);
            break;
        }

        // Cursor by Line.
        KEY_HOME {
            textbuffer_cursor_line(buffer, -i, false);
            break;
        }
        KEY_END {
            textbuffer_cursor_line(buffer, i, false);
            break;
        }
        KEY_SHIFT_HOME {
            textbuffer_cursor_line(buffer, -i, true);
            break;
        }
        KEY_SHIFT_END {
            textbuffer_cursor_line(buffer, i, true);
            break;
        }

        // Move Lines.
        KEY_CTRL_UP {
            textbuffer_edit_move_lines(buffer, -i);
            break;
        }
        KEY_CTRL_DOWN {
            textbuffer_edit_move_lines(buffer, i);
            break;
        }

        // Multi-Cursor by Row.
        KEY_SHIFT_CTRL_UP {
            textbuffer_selection_add_next_row(buffer, -i);
            break;
        }
        KEY_SHIFT_CTRL_DOWN {
            textbuffer_selection_add_next_row(buffer, i);
            break;
        }

        // Clear Multi-Cursor.
        KEY_ESC {
            textbuffer_selection_clear(buffer);
            break;
        }

        // Newline.
        KEY_ENTER {
            textbuffer_edit_newline(buffer, i);
            break;
        }
        // Delete/Backspace.
        KEY_DELETE {
            textbuffer_edit_delete(buffer, i);
            break;
        }
        KEY_ALT('d')
        KEY_BACKSPACE {
            textbuffer_edit_backspace(buffer, i);
            break;
        }
        // KEY_ALT('d'){
        //     textbuffer_edit_delete(buffer, i);
        //     break;
        // }
        KEY_ALT('D') {
            textbuffer_edit_delete_lines(buffer, i);
            break;
        }

        // Indentation.
        KEY_TAB {
            textbuffer_edit_tab(buffer, i);
            break;
        }
        KEY_SHIFT_TAB {
            textbuffer_edit_indent(buffer, -i);
            break;
        }

    }

    // Insert Character.
    if (event->type == INPUT_CHAR && event->charcode >= ' ') {
        textbuffer_edit_char(buffer, event->charcode, 1);
    }
}

uint32_t input_code (InputEvent* event) {
    if (event->type == INPUT_ALT_CHAR) {
        return ALT_MASK | event->charcode;
    } else if (event->type == INPUT_CTRL_CHAR) {
        return CTRL_MASK | event->charcode;
    } else {
        return event->type;
    }
}
