#include "textaction.h"

#include "array.h"
#include "input.h"
#include "textbuffer.h"


//
// Standard Text Actions.
//
void textaction (InputEvent* event, TextBuffer* buffer, int32_t i, Array* clipboard) {
    ON_KEY(event) {
        // CTRL-Actions.
        KEY_CTRL(event) {
            // Clipboard Actions.
            CTRL('X') {

                break;
            }
            CTRL('C') {

                break;
            }
            CTRL('V') {

                break;
            }

            // Undo/Redo.
            CTRL('Z'){
                textbuffer_undo(buffer);
                break;
            }
            CTRL('Y'){
                textbuffer_redo(buffer);
                break;
            }
        } break;

        // ALT-Actions.
        KEY_ALT(event) {
            ALT('j') {

                break;
            }
            ALT('k') {

                break;
            }
            ALT('l') {

                break;
            }
            ALT(';') {

                break;
            }
        } break;

        // Cursor By Row.
        KEY_UP {
            textbuffer_cursor_row(buffer, -1, false);
            break;
        }
        KEY_DOWN {
            textbuffer_cursor_row(buffer, 1, false);
            break;
        }
        KEY_SHIFT_UP {
            textbuffer_cursor_row(buffer, -1, true);
            break;
        }
        KEY_SHIFT_DOWN {
            textbuffer_cursor_row(buffer, 1, true);
            break;
        }

        // Cursor by Column.
        KEY_LEFT {
            textbuffer_cursor_col(buffer, -1, false);
            break;
        }
        KEY_RIGHT {
            textbuffer_cursor_col(buffer, 1, false);
            break;
        }
        KEY_SHIFT_LEFT {
            textbuffer_cursor_col(buffer, -1, true);
            break;
        }
        KEY_SHIFT_RIGHT {
            textbuffer_cursor_col(buffer, 1, true);
            break;
        }

        // Cursor by Word.
        KEY_CTRL_LEFT {
            textbuffer_cursor_word(buffer, -1, false);
            break;
        }
        KEY_CTRL_RIGHT {
            textbuffer_cursor_word(buffer, 1, false);
            break;
        }
        KEY_SHIFT_CTRL_LEFT {
            textbuffer_cursor_word(buffer, -1, true);
            break;
        }
        KEY_SHIFT_CTRL_RIGHT {
            textbuffer_cursor_word(buffer, 1, true);
            break;
        }

        // Cursor by Line.
        KEY_HOME {
            textbuffer_cursor_line(buffer, -1, false);
            break;
        }
        KEY_END {
            textbuffer_cursor_line(buffer, 1, false);
            break;
        }
        KEY_SHIFT_HOME {
            textbuffer_cursor_line(buffer, -1, true);
            break;
        }
        KEY_SHIFT_END {
            textbuffer_cursor_line(buffer, 1, true);
            break;
        }

        // Move Lines.
        KEY_CTRL_UP {
            textbuffer_edit_move_lines(buffer, -1);
            break;
        }
        KEY_CTRL_DOWN {
            textbuffer_edit_move_lines(buffer, 1);
            break;
        }

        // Multi-Cursor by Row.
        KEY_SHIFT_CTRL_UP {
            textbuffer_selection_add_next_row(buffer, -1);
            break;
        }
        KEY_SHIFT_CTRL_DOWN {
            textbuffer_selection_add_next_row(buffer, 1);
            break;
        }

        // Clear Multi-Cursor.
        KEY_ESC {
            textbuffer_selection_clear(buffer);
            break;
        }

        // Newline.
        KEY_ENTER {
            textbuffer_edit_newline(buffer, 1);
            break;
        }
        // Delete/Backspace.
        KEY_DELETE {
            textbuffer_edit_delete(buffer, 1);
            break;
        }
        KEY_BACKSPACE {
            textbuffer_edit_backspace(buffer, 1);
            break;
        }

        // Indentation.
        KEY_TAB {
            textbuffer_edit_tab(buffer, 1);
            break;
        }
        KEY_SHIFT_TAB {
            textbuffer_edit_indent(buffer, -1);
            break;
        }

    }

    // Insert Character.
    if (event->type == INPUT_CHAR && event->charcode >= ' ') {
        textbuffer_edit_char(buffer, event->charcode, 1);
    }
}
