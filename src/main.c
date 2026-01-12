#include "main.h"

#include <sys/ioctl.h>

#include "array.h"
#include "input.h"
#include "output.h"
#include "editor.h"

#include "charbuffer.h"
#include "intbuffer.h"
#include "rope.h"
#include "textbuffer.h"
#include "textview.h"


int main (int argc, char** argv) {
    //
    // Process Arguments
    //
    Array* filenames = array_create();
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Handle Option.
            continue;
        }

        // Add file to list.
        array_add(filenames, argv[i]);
    }

    //
    // Launch Editor.
    //
    output_init();

    #ifdef TATL_DEPRECATED

    struct editor editor;
    editor_init(&editor, filenames);

    InputEvent event;

    struct winsize size;
    int width = 0, height = 0;

    for (;;) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {
            bool running = editor_update(&editor, &event);
            if (!running) break;
        }

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }

        editor_draw(&editor, width, height, debug);
    }

    editor_fini(&editor);

    #else

    CharBuffer* chars = charbuffer_create();

    FILE* f = fopen("src/buffer.c", "r");
    charbuffer_read(chars, f);
    fclose(f);

    IntBuffer* data = intbuffer_create();
    intbuffer_put_text(data, 0, chars);
    charbuffer_destroy(chars);

    Rope* rope = rope_create(data);
    intbuffer_destroy(data);

    TextBuffer* buffer = textbuffer_create(rope);
    TextView* view = textview_create(buffer);

    InputEvent event;

    struct winsize size;
    int width = 0, height = 0;

    for (;;) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {
            if (event.type == INPUT_ESC) break;
            switch (event.type) {
                // Cursor-Char
                case INPUT_RIGHT:
                    textbuffer_cursor_char(buffer, 1, false);
                    break;
                case INPUT_LEFT:
                    textbuffer_cursor_char(buffer, -1, false);
                    break;
                case INPUT_SHIFT_RIGHT:
                    textbuffer_cursor_char(buffer, 1, true);
                    break;
                case INPUT_SHIFT_LEFT:
                    textbuffer_cursor_char(buffer, -1, true);
                    break;
                // Cursor-Line
                case INPUT_UP:
                    textbuffer_cursor_line(buffer, -1, false);
                    break;
                case INPUT_DOWN:
                    textbuffer_cursor_line(buffer, 1, false);
                    break;
                case INPUT_SHIFT_UP:
                    textbuffer_cursor_line(buffer, -1, true);
                    break;
                case INPUT_SHIFT_DOWN:
                    textbuffer_cursor_line(buffer, 1, true);
                    break;
                // Edit-Char
                case INPUT_CHAR:
                    if (event.charcode >= ' ') {
                        textbuffer_edit_char(buffer, event.charcode, 1);
                    } else if (event.charcode == 26) {
                        // CTRL-Z - Undo.
                        textbuffer_undo(buffer);
                    } else if (event.charcode == 25) {
                        // CTRL-Y - Redo.
                        textbuffer_redo(buffer);
                    }
                    break;
                // Edit-Newline
                case INPUT_ENTER:
                    textbuffer_edit_newline(buffer, 1);
                    break;
                // Edit-Delete
                case INPUT_DELETE:
                    textbuffer_edit_delete(buffer, 1);
                    break;
                // Edit-Backspace.
                case INPUT_BACKSPACE:
                    textbuffer_edit_backspace(buffer, 1);
                    break;
            }
        }

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }

        Box window = {0, 0, width, height};
        output_clear();
        textview_draw(view, &window, event.type == INPUT_MOUSE ? &event.m_event : NULL);
        output_frame();
    }

    textview_destroy(view);
    textbuffer_destroy(buffer);

    #endif

    array_destroy(filenames);

    output_cnorm();
    output_fini();
}
