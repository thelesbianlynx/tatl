#include "editor.h"

#include "buffer.h"
#include "input.h"
#include "output.h"

static void draw_modeline (Editor* editor, Box window);

void editor_init (Editor* editor) {
    editor->width = 0;
    editor->height = 0;

    editor->buffer = buffer_create("src/editor.c");//("cat.txt");

    editor->blink = true;

    editor->debug = 0;
}

void editor_fini (Editor* editor) {
    buffer_destroy(editor->buffer);
}


bool editor_update (Editor* editor, InputStatus status, InputState* state) {
    editor->mstate = 256;
    if (status == INPUT_NONE) return true;
    if (status == INPUT_ESC) return false;

    if (status == INPUT_CHAR) {
        if (state->charcode < 32) {
            editor->debug = state->charcode;
        } else {
            buffer_edit_char(editor->buffer, state->charcode, 1);
        }
    }

    if (status == INPUT_MOUSE_MOTION) {
        editor->mstate = state->charcode;
        editor->mx = state->x;
        editor->my = state->y;
    }

    if (status == INPUT_ENTER) {
        buffer_edit_line(editor->buffer, 1);
    }

    if (status == INPUT_BACKSPACE) {
        buffer_edit_backspace(editor->buffer, 1);
    }

    if (status == INPUT_ALT_BACKSPACE || status == INPUT_DELETE) {
        buffer_edit_delete(editor->buffer, 1);
    }

    if (status == INPUT_TAB) {
        buffer_edit_tab(editor->buffer, 1);
    }

    if (status == INPUT_SHIFT_TAB) {
        buffer_edit_indent(editor->buffer, -1);
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

    if (status == INPUT_SHIFT_UP) {
        buffer_cursor_line(editor->buffer, -1, true);
    }

    if (status == INPUT_SHIFT_DOWN) {
        buffer_cursor_line(editor->buffer,  1, true);
    }

    if (status == INPUT_SHIFT_LEFT) {
        buffer_cursor_char(editor->buffer, -1, true);
    }

    if (status == INPUT_SHIFT_RIGHT) {
        buffer_cursor_char(editor->buffer,  1, true);
    }

    if (status == INPUT_CTRL_UP) {
        buffer_edit_move_line(editor->buffer, -1);
    }

    if (status == INPUT_CTRL_DOWN) {
        buffer_edit_move_line(editor->buffer, 1);
    }

    if (status == INPUT_CTRL_LEFT) {
        buffer_cursor_word(editor->buffer, 1, -1, false);
    }

    if (status == INPUT_CTRL_RIGHT) {
        buffer_cursor_word(editor->buffer, 1, 1, false);
    }

    if (status == INPUT_SHIFT_CTRL_UP) {
        // Magic.
    }

    if (status == INPUT_SHIFT_CTRL_DOWN) {
        // Magic.
    }

    if (status == INPUT_SHIFT_CTRL_LEFT) {
        buffer_cursor_word(editor->buffer, 1, -1, true);
    }

    if (status == INPUT_SHIFT_CTRL_RIGHT) {
        buffer_cursor_word(editor->buffer, 1, 1, true);
    }

    if (status == INPUT_ALT_UP) {
        buffer_cursor_paragraph(editor->buffer, -1, false);
    }

    if (status == INPUT_ALT_DOWN) {
        buffer_cursor_paragraph(editor->buffer, 1, false);
    }

    if (status == INPUT_ALT_LEFT) {
        buffer_cursor_line_begin(editor->buffer, false);
    }

    if (status == INPUT_ALT_RIGHT) {
        buffer_cursor_line_end(editor->buffer, false);
    }

    if (status == INPUT_SHIFT_ALT_UP) {
        buffer_cursor_paragraph(editor->buffer, -1, true);
    }

    if (status == INPUT_SHIFT_ALT_DOWN) {
        buffer_cursor_paragraph(editor->buffer, 1, true);
    }

    if (status == INPUT_SHIFT_ALT_LEFT) {
        buffer_cursor_line_begin(editor->buffer, true);
    }

    if (status == INPUT_SHIFT_ALT_RIGHT) {
        buffer_cursor_line_end(editor->buffer, true);
    }

    return true;
}

void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug) {
    editor->width = width;
    editor->height = height;

    editor->blink = !editor->blink;

    output_setfg(15);
    output_setbg(0);
    output_clear();

    // Status Line.
    {
        Box box = {0, height - 2, width, 2};
        draw_modeline(editor, box);
    }

    // Buffer.
    {
        Box box = {0, 0, width, height - 2};
        buffer_draw(editor->buffer, box, editor->mstate, editor->mx, editor->my);
    }

    // Commit Frame.
    output_frame();
}

static
void draw_modeline (Editor* editor, Box window) {
    int len = window.width + 1;
    char buf[len];

    Buffer* buffer = editor->buffer;

    output_cup(window.y, window.x);
    output_setfg(15);
    output_setbg(1);
    snprintf(buf, len, " %-*s", 7, "Normal");
    output_str(buf);

    char* langmode = "Text";
    int langlen = strlen(langmode);

    int status_len = window.width - 11 - langlen;
    char status_buf[status_len];
    snprintf(status_buf, status_len, "%d:%d  %s  %s", buffer->cursor.line, buffer->cursor.col, "LN", "Spaces 4");

    output_setfg(0);
    output_setbg(7);
    snprintf(buf, len, " %s %*s ", langmode, status_len, status_buf);
    output_str(buf);

    output_cup(window.y + 1, window.x);
    output_setfg(7);
    output_setbg(8);
    //char str[512];
    //snprintf(str, 512, "%d %u %u", editor->debug, editor->dx, editor->dy);
    snprintf(buf, len, " %-*d", window.width - 1, editor->debug); //"'Message'");
    output_str(buf);
}
