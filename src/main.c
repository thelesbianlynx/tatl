#include "main.h"

#include <sys/ioctl.h>
#undef CTRL

#include "array.h"
#include "input.h"
#include "output.h"
#include "editor.h"

#include "filebuffer.h"

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

    FileBuffer* fb = filebuffer_create();
    // filebuffer_read(fb, "src/main.h");

    InputEvent event = {};

    struct winsize size;
    int width = 0, height = 0;

    bool exit = false;
    while (!exit) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {
            // exit = !editor_event(&editor, &event);
            ON_KEY(&event) {
                KEY_CTRL(&event) {
                    CTRL('Q') {
                        exit = true;
                        break;
                    }
                }
                default: {
                    textaction(&event, fb->buffer, 1, editor.clipboard);
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
        filebuffer_draw(fb, &window, event.type == INPUT_MOUSE ? &event.m_event : NULL);
        output_frame();
    }

    filebuffer_destroy(fb);

    array_destroy(filenames);

    editor_fini(&editor);

    output_cnorm();
    output_fini();
}
