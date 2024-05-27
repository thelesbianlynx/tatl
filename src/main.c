#include "main.h"

#include <sys/ioctl.h>

#include "input.h"
#include "output.h"
#include "editor.h"


int main () {
    output_init();

    struct editor editor;
    editor_init(&editor);

    bool running;

    InputStatus status;
    InputState state;

    struct winsize size;
    int width = 0, height = 0;

    for (;;) {
        uint32_t debug[32] = {0};
        status = nextkey(10, &state, debug);
        running = editor_update(&editor, status, &state);

        if (!running) break;

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }

        if (status != INPUT_NONE)
            debug[0] = 0;
        editor_draw(&editor, width, height, debug);

    }

    editor_fini(&editor);

    output_fini();
}
