#include "main.h"

#include <sys/ioctl.h>
#undef CTRL

#include "array.h"
#include "input.h"
#include "output.h"
#include "editor.h"

#include "charbuffer.h"
#include "intbuffer.h"
#include "rope.h"
#include "textbuffer.h"
#include "textview.h"
#include "textaction.h"

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

    Editor editor;
    editor_init(&editor);

    CharBuffer* chars = charbuffer_create();

    FILE* f = fopen("src/textbuffer.c", "r");
    charbuffer_read(chars, f);
    fclose(f);

    IntBuffer* data = intbuffer_create();
    intbuffer_put_text(data, 0, chars);
    charbuffer_destroy(chars);

    Rope* rope = rope_create(data);
    intbuffer_destroy(data);

    TextBuffer* buffer = textbuffer_create(rope);
    TextView* view = textview_create(buffer);

    InputEvent event = {};

    struct winsize size;
    int width = 0, height = 0;

    bool exit = false;
    while (!exit) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {

            ON_KEY(&event) {
                KEY_CTRL(&event) {
                    CTRL('Q') {
                        exit = true;
                        break;
                    }
                }
                default:{
                    textaction(&event, buffer, 1, editor.clipboard);
                    break;
                }
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

    array_destroy(filenames);

    editor_fini(&editor);

    output_cnorm();
    output_fini();
}
