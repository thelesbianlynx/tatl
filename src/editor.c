#include "editor.h"

#include "buffer.h"

#include <curses.h>
#include <term.h>

void editor_init (Editor* editor) {
    editor->width = 0;
    editor->height = 0;

    editor->buffer = buffer_create("cat.txt");

}

void editor_fini (Editor* editor) {
    buffer_destroy(editor->buffer);
}


bool editor_update (Editor* editor, InputStatus status, InputState* state) {
    if (status == INPUT_NONE) return true;

    if (status == INPUT_ESC) return false;

    if (status == INPUT_CHAR) {
        buffer_edit_char(editor->buffer, state->charcode, 1);
    }

    if (status == INPUT_ENTER) {
        buffer_edit_line(editor->buffer, 1);
    }

    if (status == INPUT_BACKSPACE) {
        buffer_edit_backspace(editor->buffer, 1);
    }

    if (status == INPUT_UP) {
        buffer_cursor_line(editor->buffer, -1, false);
    }

    if (status == INPUT_DOWN) {
        buffer_cursor_line(editor->buffer,  1, false);
    }

    if (status == INPUT_LEFT) {
        buffer_cursor_char(editor->buffer, -1, false);
    }

    if (status == INPUT_RIGHT) {
        buffer_cursor_char(editor->buffer,  1, false);
    }




    return true;
}

void editor_draw (Editor* editor, uint32_t width, uint32_t height, uint32_t* debug) {
    editor->width = width;
    editor->height = height;

    output_setfg(15);
    output_setbg(0);
    output_clear();

    Box box = {1, 1, width-2, height - 3};

    output_setfg(0);
    output_setbg(15);
    buffer_draw(editor->buffer, box);


    output_frame();

    // tputs(tparm(tigetstr("cup"), 2, 10), 1, putchar);
    // printf("Width: %d, Height: %d", width, height);
    //
    // tputs(tparm(tigetstr("cup"), 1, 10), 1, putchar);
    // tputs(tparm(tigetstr("setab"), 15), 1, putchar);
    // tputs(tparm(tigetstr("setaf"), 0), 1, putchar);
    //
    // if (editor->last_status == INPUT_CHAR) {
    //     printf("Key: %c\n", editor->last_keycode);
    // } else if (editor->last_status == INPUT_ALT_CHAR) {
    //     printf("Key: ALT+%c\n", editor->last_keycode);
    // } else if (editor->last_status == INPUT_TAB) {
    //     printf("Key: TAB\n");
    // } else if (editor->last_status == INPUT_ENTER) {
    //     printf("Key: ENTER\n");
    // } else if (editor->last_status == INPUT_ESC) {
    //     printf("Key: ESC\n");
    // } else if (editor->last_status == INPUT_BACKSPACE) {
    //     printf("Key: BACKSPACE\n");
    // } else if (editor->last_status == INPUT_UP) {
    //     printf("Key: UP\n");
    // } else if (editor->last_status == INPUT_DOWN) {
    //     printf("Key: DOWN\n");
    // } else if (editor->last_status == INPUT_LEFT) {
    //     printf("Key: LEFT\n");
    // } else if (editor->last_status == INPUT_RIGHT) {
    //     printf("Key: RIGHT\n");
    // } else if (editor->last_status == INPUT_SHIFT_UP) {
    //     printf("Key: SHIFT+UP\n");
    // } else if (editor->last_status == INPUT_SHIFT_DOWN) {
    //     printf("Key: SHIFT+DOWN\n");
    // } else if (editor->last_status == INPUT_SHIFT_LEFT) {
    //     printf("Key: SHIFT+LEFT\n");
    // } else if (editor->last_status == INPUT_SHIFT_RIGHT) {
    //     printf("Key: SHIFT+RIGHT\n");
    // } else if (editor->last_status == INPUT_SHIFT_TAB) {
    //     printf("Key: SHIFT+TAB\n");
    // } else if (editor->last_status == INPUT_ALT_ENTER) {
    //     printf("Key: ALT+ENTER\n");
    // } else if (editor->last_status == INPUT_ALT_BACKSPACE) {
    //     printf("Key: ALT+BACKSPACE\n");
    // }
    //
    // tputs(tparm(tigetstr("cup"), 3, 10), 1, putchar);
    // tputs(tparm(tigetstr("setab"), 15), 1, putchar);
    // tputs(tparm(tigetstr("setaf"), 0), 1, putchar);
    //
    // if (debug[0] != 0) {
    //     printf("Debug:");
    //     for (int i = 0; i < debug[0] && i < 30; ++i) {
    //         printf("%d ", debug[i+1]);
    //     }
    // }
    //
    // fflush(stdout);
}
