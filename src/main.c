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
    editor_init(&editor, filenames);

    InputEvent event = {};

    struct winsize size;
    int width = 0, height = 0;

    bool exit = false;
    while (!exit) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {
            exit = !editor_event(&editor, &event);
        }

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }

        Box window = {0, 0, width, height};
        output_clear();
        editor_draw(&editor, &window, event.type == INPUT_MOUSE ? &event.m_event : NULL);
        output_frame();
    }

    array_destroy(filenames);

    editor_fini(&editor);

    output_cnorm();
    output_fini();
}
