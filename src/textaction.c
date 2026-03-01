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
        KEY_BACKSPACE {
            textbuffer_edit_backspace(buffer, i);
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
