#include "main.h"

#include <curses.h>
#include <term.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "input.h"
#include "editor.h"


int main () {
    setupterm(NULL, 1, NULL);

    struct termios termios;
    tcgetattr(0, &termios);
    {
        struct termios term_raw = termios;
        cfmakeraw(&term_raw);
        tcsetattr(0, TCSANOW, &term_raw);
    }

    tputs(tigetstr("smcup"), 1, putchar);

    struct editor editor;
    editor_init(&editor);

    bool running;

    uint32_t status;
    struct input_state state;

    struct winsize size;
    int width = 0, height = 0;

    for (;;) {
        status = nextkey(10, &state);
        running = editor_update(&editor, status, &state);

        if (!running) break;

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }

        editor_draw(&editor, width, height);

    }

    editor_fini(&editor);

    tputs(tigetstr("rmcup"), 1, putchar);

    tcsetattr(0, TCSANOW, &termios);
}
