#include "editor.h"

#include "actions.h"
#include "array.h"
#include "buffer.h"
#include "charbuffer.h"
#include "input.h"
#include "output.h"

static void draw_modeline (Editor* editor, Box window);

void editor_init (Editor* editor) {
    editor->buffers = array_create();
    editor->buffer_id = 0;
    editor->clipboard = charbuffer_create();
    editor->mstate = 256;
    editor->mx = 0;
    editor->my = 0;
    editor->debug = 0;

    array_add(editor->buffers, buffer_create("Untitled"));
}

void editor_fini (Editor* editor) {
    array_destroy(editor->buffers);
}


//
// Update (called on input event).
//
bool editor_update (Editor* editor, InputEvent* event) {
    if (editor->buffers->size == 0) return false;

    Buffer* buffer = editor->buffers->data[editor->buffer_id];

    if (event->type == INPUT_ESC) {
        // Escape: Exit Editor.
        return false;
    } else if (event->type == INPUT_CHAR) {
        // Character Event.
        uint32_t c = event->charcode;
        if (c >= 32) {
            // Insert Character.
            buffer_edit_char(buffer, c, 1);
        } else {
            // CTRL-Action.
            action_fn a = actions[c];
            if (a) a(editor, buffer, 1);
        }
    } else if (event->type == INPUT_ALT_CHAR) {
        // ALT-Character Event.
        uint32_t c = event->charcode;
        if (c < MAX_ACTIONS) {
            // ALT-Action.
            action_fn a = actions[c];
            if (a) a(editor, buffer, 1);
        }
    } else if (event->type == INPUT_MOUSE) {
        // Mouse Event.
        editor->mstate = event->charcode;
        editor->mx = event->x;
        editor->my = event->y;
    } else {
        // Else: Fixed-Function Action.
        action_fn a = fixed_actions[event->type];
        if (a) a(editor, buffer, 1);
    }




    // if (status == INPUT_ESC) {
    //     if (editor->edit_mode) {
    //         editor_escape(editor);
    //     } else {
    //         return false;
    //     }
    // } else if (status == INPUT_CHAR) {
    //     if (editor->edit_mode && state->charcode > 32) {
    //         buffer_edit_char(buffer, state->charcode, 1);
    //     } else if (state->charcode < MAX_ACTIONS) {
    //         action_fn a = actions[state->charcode];
    //         if (a) a(editor, buffer, 1);
    //     }
    // } else if (status == INPUT_ALT_CHAR) {
    //     if (state->charcode < MAX_ALT_ACTIONS) {
    //         action_fn alt_a = alt_actions[state->charcode];
    //         if (alt_a) alt_a(editor, buffer, 1);
    //         else if (state->charcode < MAX_ACTIONS) {
    //             action_fn a = actions[state->charcode];
    //             if (a) a(editor, buffer, 1);
    //         }
    //     }
    // }  else {
    //     action_fn a = fixed_actions[status];
    //     if (a) a(editor, buffer, 1);
    // }

    return true;
}

//
// Draw.
//
void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug) {
    editor->buffer_id = MOD(editor->buffer_id, editor->buffers->size);
    Buffer* buffer = editor->buffers->data[editor->buffer_id];

    output_normal();
    output_clear();

    // Status Line.
    // {
    //     Box box = {0, height - 2, width, 2};
    //     draw_modeline(editor, box);
    // }

    // Buffer.
    {
        Box box = {0, 1, width, height - 2};
        buffer_draw(buffer, box, editor->mstate, editor->mx, editor->my);
    }

    // Commit Frame.
    output_frame();

    editor->mstate = 256;
}


//
// Clipboard
//
CharBuffer* editor_get_clipboard (Editor* editor) {
    return editor->clipboard;
}

//
// File Operations.
//
void editor_new (Editor* editor) {
    Buffer* buffer = buffer_create("Untitled");
    array_insert(editor->buffers, editor->buffer_id + 1, buffer);
    editor_buffer_next(editor);
}

void editor_open (Editor* editor) {

}

void editor_save (Editor* editor) {

}

void editor_save_all (Editor* editor) {

}

void editor_save_as (Editor* editor) {

}

//
// Window Operations.
//
void editor_close (Editor* editor) {

}

void editor_quit (Editor* editor) {

}

void editor_buffer_next (Editor* editor) {
    editor->buffer_id = MOD(editor->buffer_id + 1, editor->buffers->size);
}

void editor_buffer_prev (Editor* editor) {
    editor->buffer_id = MOD(editor->buffer_id - 1, editor->buffers->size);
}


static
void draw_modeline (Editor* editor, Box window) {
    int len = window.width + 1;
    char buf[len];

    editor->buffer_id = MOD(editor->buffer_id, editor->buffers->size);
    Buffer* buffer = editor->buffers->data[editor->buffer_id];

    output_cup(window.y, window.x);
    //output_setfg(15);
    //if (!editor->edit_mode) {
        //output_setbg(1);
        snprintf(buf, len, " %-*s", 7, "Normal");
        output_str(buf);
    // } else {
    //     //output_setbg(4);
    //     snprintf(buf, len, " %-*s", 7, " Edit ");
    //     output_str(buf);
    // }

    char* langmode = "Text";
    int langlen = strlen(langmode);

    int status_len = window.width - 11 - langlen;
    char status_buf[status_len];
    snprintf(status_buf, status_len, "%d/%d   %d:%d  %s  %s", editor->buffer_id + 1, editor->buffers->size,
        buffer->cursor.line, buffer->cursor.col, "LN", "Spaces(4)");

    //output_setfg(0);
    //output_setbg(7);
    snprintf(buf, len, " %s %*s ", langmode, status_len, status_buf);
    output_str(buf);

    output_cup(window.y + 1, window.x);
    //output_setfg(7);
    //output_setbg(8);
    //char str[512];
    //snprintf(str, 512, "%d %u %u", editor->debug, editor->dx, editor->dy);
    snprintf(buf, len, " %-*d", window.width - 1, editor->debug); //"'Message'");
    output_str(buf);
}
