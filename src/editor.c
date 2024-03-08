#include "editor.h"

#include <curses.h>
#include <term.h>


void editor_init (struct editor* editor) {
    editor->width = 0;
    editor->height = 0;

    editor->last_status = INPUT_NONE;
    editor->last_keycode = 0;
}

void editor_fini (struct editor* editor) {
    // ...
}


bool editor_update (struct editor* editor, uint32_t status, struct input_state* state) {
    if (status == INPUT_NONE) return true;

    if (status == INPUT_CHAR || status == INPUT_ALT_CHAR) editor->last_keycode = state->charcode;

    if (editor->last_keycode == 'q') return false;

    editor->last_status = status;

    return true;
}

void editor_draw (struct editor* editor, uint32_t width, uint32_t height) {
    editor->width = width;
    editor->height = height;

    tputs(tparm(tigetstr("setab"), 0), 1, putchar);
    tputs(tparm(tigetstr("setaf"), 15), 1, putchar);
    tputs(tigetstr("clear"), 1, putchar);

    tputs(tparm(tigetstr("cup"), 5, 10), 1, putchar);
    printf("Width: %d, Height: %d", width, height);

    tputs(tparm(tigetstr("cup"), 4, 10), 1, putchar);
    tputs(tparm(tigetstr("setab"), 15), 1, putchar);
    tputs(tparm(tigetstr("setaf"), 0), 1, putchar);

    if (editor->last_status == INPUT_CHAR) {
        printf("Key: %c\n", editor->last_keycode);
    } else if (editor->last_status == INPUT_ALT_CHAR) {
        printf("Key: ALT+%c\n", editor->last_keycode);
    } else if (editor->last_status == INPUT_TAB) {
        printf("Key: TAB\n");
    } else if (editor->last_status == INPUT_ENTER) {
        printf("Key: ENTER\n");
    } else if (editor->last_status == INPUT_ESC) {
        printf("Key: ESC\n");
    } else if (editor->last_status == INPUT_BACKSPACE) {
        printf("Key: BACKSPACE\n");
    } else if (editor->last_status == INPUT_UP) {
        printf("Key: UP\n");
    } else if (editor->last_status == INPUT_DOWN) {
        printf("Key: DOWN\n");
    } else if (editor->last_status == INPUT_LEFT) {
        printf("Key: LEFT\n");
    } else if (editor->last_status == INPUT_RIGHT) {
        printf("Key: RIGHT\n");
    }

    fflush(stdout);
}
