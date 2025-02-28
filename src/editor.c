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
