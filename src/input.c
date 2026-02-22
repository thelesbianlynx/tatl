#include "input.h"

#include <poll.h>
#include <unistd.h>

static bool first_run = true;

static
void parseCSI (const char* buffer, int n, char* ch, uint32_t* keycode, uint32_t* mods, uint32_t* button) {
    char b1[n];
    char b2[n];
    char b3[n];
    memset(b1, 0, n);
    memset(b2, 0, n);
    memset(b3, 0, n);

    int i = 0;
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
                // Only two arguments, early return.
                if (i >= n - 1) {
                    *ch = buffer[n-1];
                    *keycode = atoi(b1);
                    *mods = atoi(b2);
                    return;
                }

                // Third argument.
                if (buffer[i] == ';') {
                    if (button == NULL) {
                        *ch = buffer[n-1];
                        *keycode = atoi(b1);
                        *mods = atoi(b2);
                        return;
                    }

                    j = 0;
                    i = i + 1;

                    for (;;) {
                        if (i >= n - 1 || buffer[i] == ';') {
                            *ch = buffer[n-1];
                            *button = atoi(b1);
                            *keycode = atoi(b2);
                            *mods = atoi(b3);
                            return;
                        }

                        b3[j] = buffer[i];
                        ++i;
                        ++j;
                    }
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
    return (mods > 0 && mods % 2 == 0);
}

static
bool mod_alt (uint32_t mods) {
    return mods == 3 || mods == 4 || mods == 7 || mods == 8;
}

static
bool mod_ctrl (uint32_t mods) {
    return mods >= 5;
}

bool nextkey (int32_t timeout, InputEvent* event, int32_t* debug) {
    if (first_run) {
        first_run = false;
        return false;
    }

    struct pollfd pollfd = { .fd = 0, .events = POLLIN };
    int status = poll(&pollfd, 1, 1000);
    if (status > 0) {
        char buffer[64];
        int n = read(0, buffer, 64);
        debug[0] = n;

        if (n == 1) {
            char c = buffer[0];
            if (c == 9) {
                event->type = INPUT_TAB;
            } else if (c == 13) {
                event->type = INPUT_ENTER;
            } else if (c == 27) {
                event->type = INPUT_ESC;
            } else if (c == 127) {
                event->type = INPUT_BACKSPACE;
            } else {
                event->type = INPUT_CHAR;
                event->charcode = c;
            }
            return true;
        }

        if (n == 2 && buffer[0] == '\e') {
            char c = buffer[1];
            if (c == 13) {
                event->type = INPUT_ALT_ENTER;
            } else if (c == 127) {
                event->type = INPUT_ALT_BACKSPACE;
            } else if (c >= 32) {
                event->type = INPUT_ALT_CHAR;
                event->charcode = c;
            } else {
                return false;
            }
            return true;
        }

        if (n >= 3 && buffer[0] == '\e' && buffer[1] == '[') {
            if (buffer[2] == 'M' && n >= 6) {
                // Mouse Event.
                event->type = INPUT_MOUSE;
                event->m_event.button = buffer[3] - 32;
                event->m_event.x = (uint8_t) buffer[4] - 32;
                event->m_event.y = (uint8_t) buffer[5] - 32;
                return true;
            }

            if (buffer[2] == '<') {
                // SGR-Mouse Event.
                char c;
                uint32_t x, y, button;
                parseCSI(buffer + 3, n - 3, &c, &x, &y, &button);
                if (c != 'm') {
                    event->type = INPUT_MOUSE;
                    event->m_event.button = button;
                    event->m_event.x = x;
                    event->m_event.y = y;
                    return true;
                }
                return false;
            }

            // for (int i = 0; i < n && i < 30; ++i) {
            //     debug[i+1] = buffer[i];
            // }

            char c;
            uint32_t key, mods;
            parseCSI(buffer + 2, n - 2, &c, &key, &mods, NULL);

            if (c == '~') {
                // Mods & Key appear to be Backwards.
                //  Fix Later...
                if (key == 1 && mods == 3) {
                    event->type = INPUT_DELETE;
                    return true;
                } else if (key == 1 && mods == 1) {
                    event->type = INPUT_HOME;
                    return true;
                } else if (key == 1 && mods == 4) {
                    event->type = INPUT_END;
                    return true;
                } else if (key == 2 && mods == 1) {
                    event->type = INPUT_SHIFT_HOME;
                    return true;
                } else if (key == 2 && mods == 4) {
                    event->type = INPUT_SHIFT_END;
                    return true;
                } else {
                    return false;
                }
            }

            if (c == 'u') {
                // Kitty input event.
                // ...

                return false;
            }

            if (mod_ctrl(mods)) {
                if (mod_shift(mods)) {
                    if (c == 'A') {
                        event->type = INPUT_SHIFT_CTRL_UP;
                    } else if (c == 'B') {
                        event->type = INPUT_SHIFT_CTRL_DOWN;
                    } else if (c == 'D') {
                        event->type = INPUT_SHIFT_CTRL_LEFT;
                    } else if (c == 'C') {
                        event->type = INPUT_SHIFT_CTRL_RIGHT;
                    } else {
                        return false;
                    }
                    return true;
                }

                if (mod_alt(mods)) {
                    if (c == 'A') {
                        event->type = INPUT_CTRL_ALT_UP;
                    } else if (c == 'B') {
                        event->type = INPUT_CTRL_ALT_DOWN;
                    } else if (c == 'D') {
                        event->type = INPUT_CTRL_ALT_LEFT;
                    } else if (c == 'C') {
                        event->type = INPUT_CTRL_ALT_RIGHT;
                    } else {
                        return false;
                    }
                    return true;
                }

                if (c == 'A') {
                    event->type = INPUT_CTRL_UP;
                } else if (c == 'B') {
                    event->type = INPUT_CTRL_DOWN;
                } else if (c == 'D') {
                    event->type = INPUT_CTRL_LEFT;
                } else if (c == 'C') {
                    event->type = INPUT_CTRL_RIGHT;
                } else {
                    return false;
                }
                return true;
            }

            if (mod_alt(mods)) {
                if (mod_shift(mods)) {
                    if (c == 'A') {
                        event->type = INPUT_SHIFT_ALT_UP;
                    } else if (c == 'B') {
                        event->type = INPUT_SHIFT_ALT_DOWN;
                    } else if (c == 'D') {
                        event->type = INPUT_SHIFT_ALT_LEFT;
                    } else if (c == 'C') {
                        event->type = INPUT_SHIFT_ALT_RIGHT;
                    } else {
                        return false;
                    }
                    return true;
                }

                if (c == 'A') {
                    event->type = INPUT_ALT_UP;
                } else if (c == 'B') {
                    event->type = INPUT_ALT_DOWN;
                } else if (c == 'D') {
                    event->type = INPUT_ALT_LEFT;
                } else if (c == 'C') {
                    event->type = INPUT_ALT_RIGHT;
                } else {
                    return false;
                }
                return true;
            }

            if (mod_shift(mods)) {
                if (c == 'A') {
                    event->type = INPUT_SHIFT_UP;
                } else if (c == 'B') {
                    event->type = INPUT_SHIFT_DOWN;
                } else if (c == 'D') {
                    event->type = INPUT_SHIFT_LEFT;
                } else if (c == 'C') {
                    event->type = INPUT_SHIFT_RIGHT;
                } else if (c == 'H') {
                    event->type = INPUT_SHIFT_HOME;
                } else if (c == 'F') {
                    event->type = INPUT_SHIFT_END;
                } else {
                    return false;
                }
                return true;
            }

            if (c == 'A') {
                event->type = INPUT_UP;
            } else if (c == 'B') {
                event->type = INPUT_DOWN;
            } else if (c == 'D') {
                event->type = INPUT_LEFT;
            } else if (c == 'C') {
                event->type = INPUT_RIGHT;
            } else if (c == 'H') {
                event->type = INPUT_HOME;
            } else if (c == 'F') {
                event->type = INPUT_END;
            } else if (c == 'Z') {
                event->type = INPUT_SHIFT_TAB;
            } else {
                return false;
            }

            return true;
        }
    }

    return false;
}
