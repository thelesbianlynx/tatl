#include "main.h"

#include <sys/ioctl.h>

#include "array.h"
#include "input.h"
#include "output.h"
#include "editor.h"


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
    #endif

    output_cnorm();
    output_fini();
}
