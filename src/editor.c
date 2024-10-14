#include "editor.h"

#include "buffer.h"
#include "charbuffer.h"
#include "actions.h"
#include "input.h"
#include "output.h"

static void draw_modeline (Editor* editor, Box window);

void editor_init (Editor* editor) {
    editor->buffer = buffer_create("src/editor.c");//("cat.txt");
    editor->mode = MODE_NORMAL;
    editor->clipboard = charbuffer_create();
    editor->blink = true;
    editor->debug = 0;
    editor->mstate = 256;
    editor->mx = 0;
    editor->my = 0;
}

void editor_fini (Editor* editor) {
    buffer_destroy(editor->buffer);
}

bool editor_update (Editor* editor, InputStatus status, InputState* state) {
    editor->mstate = 256;

    Buffer* buffer = editor->buffer;

    if (status == INPUT_ESC) {
        if (editor->mode == MODE_EDIT) {
            editor_exit_mode(editor);
        } else {
            return false;
        }
    } else if (status == INPUT_CHAR) {
        if (editor->mode == MODE_EDIT && state->charcode >= 32) {
            buffer_edit_char(editor->buffer, state->charcode, 1);
        } else if (state->charcode < MAX_ACTIONS) {
            action_fn a = actions[state->charcode];
            if (a) a(editor, buffer, 1);
        }
    } else if (status == INPUT_ALT_CHAR) {
        if (state->charcode < MAX_ALT_ACTIONS) {
            action_fn alt_a = alt_actions[state->charcode];
            if (alt_a) alt_a(editor, buffer, 1);
            else {
                action_fn a = actions[state->charcode];
                if (a) a(editor, buffer, 1);
            }
        }
    } else if (status == INPUT_MOUSE_MOTION) {
        editor->mstate = state->charcode;
        editor->mx = state->x;
        editor->my = state->y;
    } else {
        action_fn a = fixed_actions[status];
        if (a) a(editor, buffer, 1);
    }

    return true;
}

void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug) {
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

void editor_enter_mode (Editor* editor, uint32_t mode) {
    if (mode == MODE_EDIT) {
        editor->mode = MODE_EDIT;
    }
}

void editor_exit_mode (Editor* editor) {
    editor->mode = MODE_NORMAL;
}

CharBuffer* editor_get_clipboard (Editor* editor) {
    return editor->clipboard;
}


static
void draw_modeline (Editor* editor, Box window) {
    int len = window.width + 1;
    char buf[len];

    Buffer* buffer = editor->buffer;

    output_cup(window.y, window.x);
    output_setfg(15);
    if (editor->mode == MODE_NORMAL) {
        output_setbg(1);
        snprintf(buf, len, " %-*s", 7, "Normal");
        output_str(buf);
    } else if (editor->mode == MODE_EDIT) {
        output_setbg(4);
        snprintf(buf, len, " %-*s", 7, " Edit ");
        output_str(buf);
    }

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
