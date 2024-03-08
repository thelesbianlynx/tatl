#include "input.h"

#include <poll.h>
#include <unistd.h>

uint32_t nextkey (int32_t timeout, struct input_state* r_inputstate) {
    struct pollfd pollfd = { .fd = 0, .events = POLLIN };
    int status = poll(&pollfd, 1, 10);
    if (status > 0) {
        char buffer[64];
        int n = read(0, buffer, 64);

        if (n == 1) {
            char c = buffer[0];
            if (c == 9)  return INPUT_TAB;
            if (c == 13)  return INPUT_ENTER;
            if (c == 27)  return INPUT_ESC;
            if (c == 127) return INPUT_BACKSPACE;
            if (c >= 32) {
                r_inputstate->charcode = c;
                return INPUT_CHAR;
            }
        }

        if (n == 2 && buffer[0] == '\e') {
            char c = buffer[1];
            if (c >= 32) {

                r_inputstate->charcode = c;
                return INPUT_ALT_CHAR;
            }
        }

        if (n >= 3 && buffer[0] == '\e' && buffer[1] == '[') {
            if (buffer[2] == 'A') return INPUT_UP;
            if (buffer[2] == 'B') return INPUT_DOWN;
            if (buffer[2] == 'D') return INPUT_LEFT;
            if (buffer[2] == 'C') return INPUT_RIGHT;


        }
    }

    return INPUT_NONE;
}
