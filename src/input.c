#include "input.h"

#include <poll.h>
#include <unistd.h>

static
void parseCSI (const char* buffer, int n, char* ch, uint32_t* keycode, uint32_t* mods) {
    char b1[n] = {};
    char b2[n] = {};

    int i = 2;
    int j = 0;

    // First argument.
    for (;;) {
        // Only one argument, early return.
        if (i >= n - 1) {
            *ch = buffer[n-1];
            *keycode = 1;
            *mods = atoi(b1);
            return;
        }

        // Second argument.
        if (buffer[i] == ';') {
            j = 0;
            i = i + 1;
            for (;;) {
                if (i >= n - 1 || buffer[i] == ';') {
                    *ch = buffer[n-1];
                    *keycode = atoi(b1);
                    *mods = atoi(b2);
                    return;
                }

                b2[j] = buffer[i];
                ++i;
                ++j;
            }
        }

        b1[j] = buffer[i];
        ++i;
        ++j;
    }
}

static
bool mod_shift (uint32_t mods) {
    return (mods > 0 && mods % 2 == 0) || mods == 5;
}

uint32_t nextkey (int32_t timeout, struct input_state* r_inputstate, uint32_t* debug) {
    struct pollfd pollfd = { .fd = 0, .events = POLLIN };
    int status = poll(&pollfd, 1, 1000);
    if (status > 0) {
        char buffer[64];
        int n = read(0, buffer, 64);
        debug[0] = n;

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
            if (c == 13) return INPUT_ALT_ENTER;
            if (c == 127) return INPUT_ALT_BACKSPACE;
            if (c >= 32) {
                r_inputstate->charcode = c;
                return INPUT_ALT_CHAR;
            }
        }

        if (n >= 3 && buffer[0] == '\e' && buffer[1] == '[') {
            if (buffer[2] == 'M') {
                // Mouse Event.
                // ...
                return INPUT_NONE;
            }

            for (int i = 0; i < n && i < 30; ++i) {
                debug[i+1] = buffer[i];
            }

            char c;
            uint32_t key, mods;
            parseCSI(buffer, n, &c, &key, &mods);

            if (c == '~') {
                // Complicated.
                // ...

                return INPUT_NONE;
            }

            if (c == 'u') {
                // Kitty input event.
                // ...

                return INPUT_NONE;
            }

            if (c == 'A' && mod_shift(mods)) return INPUT_SHIFT_UP;
            if (c == 'B' && mod_shift(mods)) return INPUT_SHIFT_DOWN;
            if (c == 'D' && mod_shift(mods)) return INPUT_SHIFT_LEFT;
            if (c == 'C' && mod_shift(mods)) return INPUT_SHIFT_RIGHT;

            if (c == 'A') return INPUT_UP;
            if (c == 'B') return INPUT_DOWN;
            if (c == 'D') return INPUT_LEFT;
            if (c == 'C') return INPUT_RIGHT;

            if (c == 'Z') return INPUT_SHIFT_TAB;


        }
    }

    return INPUT_NONE;
}
