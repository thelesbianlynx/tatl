#include "editor.h"

#include "actions.h"
#include "array.h"
#include "buffer.h"
#include "charbuffer.h"
#include "input.h"
#include "output.h"

static void update_tab_bar (Editor* editor, Box box);
static void draw_tab_bar (Editor* editor, Box box, uint32_t mstate, uint32_t mx, uint32_t my);

enum alt_modes {
    NORMAL = 0,
    ALT_OPEN,
    ALT_SAVE,
    ALT_CLOSE,
    ALT_FIND,
    ALT_REPLACE,
    ALT_COMMAND,
};


void editor_init (Editor* editor) {
    editor->buffers = array_create();
    editor->buffer_id = 0;
    editor->tab_bar = charbuffer_create();
    editor->tab_scroll = 0;
    editor->clipboard = charbuffer_create();
    editor->tab_scroll_dmg = true;
    editor->alt_mode = NORMAL,
    editor->alt_buffer = buffer_create(editor, "");
    editor->alt_buffer->alt_mode = true;
    editor->target_buffer = NULL;
    editor->mstate = 256;
    editor->mx = 0;
    editor->my = 0;


    array_add(editor->buffers, buffer_create(editor, "Untitled"));
}

void editor_fini (Editor* editor) {
    array_destroy(editor->buffers);
}


static inline
Buffer* get_buffer (Editor* editor) {
    editor->buffer_id = MOD(editor->buffer_id, editor->buffers->size);
    Buffer* buffer = editor->buffers->data[editor->buffer_id];
    return buffer;
}




//
// Update (called on input event).
//
bool editor_update (Editor* editor, InputEvent* event) {
    if (editor->buffers->size == 0) return false;

    Buffer* buffer = editor->alt_mode == NORMAL ? get_buffer(editor) : editor->alt_buffer;

    if (event->type == INPUT_ESC) {
        // Escape: Exit Editor.
        return false;
    } else if (event->type == INPUT_CHAR) {
        // Character Event.
        uint32_t c = event->charcode;
        if (c >= 32 && c != 127) {
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
    output_normal();
    output_clear();

    // Tab Bar.
    {
        Box box = {0, 0, width, 3};
        // if (editor->tab_damage) {
            update_tab_bar(editor, box);
        // }
        draw_tab_bar(editor, box, editor->mstate, editor->mx, editor->my);
    }

    // Buffer.
    {
        Buffer* buffer = get_buffer(editor);

        Box box = {0, 3, width, height - 4};
        buffer_draw(buffer, box, editor->mstate, editor->mx, editor->my);
    }

    // Alt-Buffer.
    if (editor->alt_mode != NORMAL) {
        Box box = {0, height - 1, width, 1};
        buffer_draw(editor->alt_buffer, box, editor->mstate, editor->mx, editor->my);
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
Buffer* editor_new (Editor* editor) {
    Buffer* buffer = buffer_create(editor, "Untitled");
    array_insert(editor->buffers, editor->buffer_id + 1, buffer);
    editor->buffer_id = MOD(editor->buffer_id + 1, editor->buffers->size);
    editor->tab_scroll_dmg = true;
    return buffer;
}

void editor_open (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;

    editor->alt_mode = ALT_OPEN;
    editor->target_buffer = get_buffer(editor);
    buffer_prompt(editor->alt_buffer, "");
    buffer_title(editor->alt_buffer, "(Open)");
}

void editor_save (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;

    Buffer* buffer = get_buffer(editor);
    if (!buffer_save(buffer)) {
        editor->alt_mode = ALT_SAVE;
        editor->target_buffer = get_buffer(editor);
        buffer_prompt(editor->alt_buffer, buffer->filename->buffer); // This Line is bad!!
        buffer_title(editor->alt_buffer, "(Save)");
    }
}

void editor_save_all (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;
}

void editor_save_as (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;

    Buffer* buffer = get_buffer(editor);
    editor->alt_mode = ALT_SAVE;
    editor->target_buffer = buffer;
    buffer_prompt(editor->alt_buffer, buffer->filename->buffer);
    buffer_title(editor->alt_buffer, "(Save)");
}

//
// Window Operations.
//
void editor_close (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;

}

void editor_quit (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;

}

void editor_buffer_next (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;
    editor->buffer_id = MOD(editor->buffer_id + 1, editor->buffers->size);
    editor->tab_scroll_dmg = true;
}

void editor_buffer_prev (Editor* editor) {
    if (editor->alt_mode != NORMAL) return;
    editor->buffer_id = MOD(editor->buffer_id - 1, editor->buffers->size);
    editor->tab_scroll_dmg = true;
}


//
// Alt-Buffer Callbacks.
//

void editor_altbuffer_enter (Editor* editor) {
    Buffer* buffer = editor->target_buffer;
    if (buffer == NULL) return;

    if (editor->alt_mode == ALT_SAVE) {
        CharBuffer* savename = charbuffer_create();
        buffer_get_contents(editor->alt_buffer, savename);

        buffer_save_as(buffer, savename->buffer);
    }
    else if (editor->alt_mode == ALT_OPEN) {
        if (!buffer_empty(buffer)) {
            buffer = editor_new(editor);
        }

        CharBuffer* openname = charbuffer_create();
        buffer_get_contents(editor->alt_buffer, openname);

        buffer_load(buffer, openname->buffer);
    }

    editor->alt_mode = NORMAL;
    editor->target_buffer = NULL;
}

void editor_altbuffer_tab (Editor* editor) {

}

void editor_altbuffer_up (Editor* editor) {

}

void editor_altbuffer_down (Editor* editor) {

}

//
// Tab Bar.
//

static
void update_tab_bar (Editor* editor, Box box) {
    charbuffer_clear(editor->tab_bar);

    int sum = 0;
    for (int i = 0; i < editor->buffers->size; ++i) {
        Buffer* buffer = editor->buffers->data[i];
        sum += buffer->title->size + 3;
    }

    if (sum > box.width) {
        for (int i = 0; i < editor->buffers->size; ++i) {
            Buffer* buffer = editor->buffers->data[i];
            charbuffer_achar(editor->tab_bar, ' ');
            charbuffer_achars(editor->tab_bar, buffer->title);
            charbuffer_achar(editor->tab_bar, ' ');
            if (i < editor->buffers->size - 1) {
                charbuffer_achar(editor->tab_bar, '\n');
            }
        }
    } else {
        int rem = 64 * (box.width - sum);
        int each = rem / editor->buffers->size;

        for (int i = 0; i < editor->buffers->size; ++i) {
            Buffer* buffer = editor->buffers->data[i];
            charbuffer_achar(editor->tab_bar, ' ');
            charbuffer_achars(editor->tab_bar, buffer->title);
            if (i < editor->buffers->size - 1) {
                for (int i = 0; i < each; i += 64) {
                    if (rem > 0) {
                        charbuffer_achar(editor->tab_bar, ' ');
                        rem -= 64;
                    }
                }
                charbuffer_astr(editor->tab_bar, " \n");
            } else {
                while (editor->tab_bar->size < box.width) {
                    charbuffer_achar(editor->tab_bar, ' ');
                }
            }
        }
    }

    //editor->tab_scroll_dmg = true;
}

static
void draw_tab_bar (Editor* editor, Box box, uint32_t mstate, uint32_t mx, uint32_t my) {
    int bid_start = 0;
    for (int i = 0; i < editor->tab_scroll; ++i) {
        if (editor->tab_bar->buffer[i] == '\n') bid_start++;
    }

    // Mouse Input.
    if (mx > box.x && my > box.y) {
        mx -= box.x;
        my -= box.y;
        if (mx <= box.width && my <= box.height) {
            if (mstate == 0) {
                // Press.
                int bid = bid_start;
                for (int i = 0; i < mx; ++i) {
                    if (editor->tab_bar->buffer[i + editor->tab_scroll] == '\n') bid++;
                }
                editor->buffer_id = MOD(bid, editor->buffers->size);
                editor->tab_scroll_dmg = true;
            } else if (mstate == 32) {
                // Drag.

            } else if (mstate == 64) {
                // Scroll Up.
                editor->tab_scroll--;
                if (editor->tab_scroll >= 0 && editor->tab_bar->buffer[editor->tab_scroll] == '\n') bid_start--;
            } else if (mstate == 65) {
                // Scroll Down.
                if (editor->tab_bar->buffer[editor->tab_scroll] == '\n') bid_start++;
                editor->tab_scroll++;
            }
        }
    }

    // Scroll damage.
    if (editor->tab_scroll_dmg) {
        if (editor->tab_bar->size > box.width) {
            int current = MOD(editor->buffer_id, editor->buffers->size);;
            if (current <= bid_start) {
                int bid = 0;
                for (int i = 0; i < box.width && i < editor->tab_bar->size; ++i) {
                    if (editor->tab_bar->buffer[i] == '\n') bid++;
                    if (bid == current) {
                        editor->tab_scroll = i;
                        break;
                    }
                }
                bid_start = current;
            } else {
                int bid = bid_start;
                for (int i = editor->tab_scroll; i < editor->tab_bar->size; ++i) {
                    if (editor->tab_bar->buffer[i] == '\n') {
                        bid++;
                        if (bid == current) {
                            Buffer* buf = editor->buffers->data[current];
                            int size = buf->title->size;
                            while (i + size - editor->tab_scroll + 3 > box.width) {
                                if (editor->tab_bar->buffer[editor->tab_scroll] == '\n') bid_start++;
                                editor->tab_scroll++;
                            }
                        }
                    }
                }
            }
        }
        editor->tab_scroll_dmg = false;
    }

    if (editor->tab_scroll < 0) {
        editor->tab_scroll = 0;
    }
    if (editor->tab_scroll > editor->tab_bar->size - box.width)  {
        editor->tab_scroll = editor->tab_bar->size - box.width;
    }

    // Top Line
    output_normal();
    output_cup(box.y, box.x);
    output_altchar_on();
    for (int i = 0; i < box.width && i < editor->tab_bar->size; ++i) {
        if (editor->tab_bar->buffer[i + editor->tab_scroll] == '\n') {
            output_char('w');
        } else {
            output_char(ALTCHAR_HLINE);
        }
    }
    //
    // Bottom Line.
    output_cup(box.y + 2, box.x);
    for (int i = 0; i < box.width && i < editor->tab_bar->size; ++i) {
        if (editor->tab_bar->buffer[i + editor->tab_scroll] == '\n') {
            output_char('v');
        } else {
            output_char(ALTCHAR_HLINE);
        }
    }
    output_altchar_off();
    //
    // Main Line.
    output_cup(box.y + 1, box.x);
    int bid = bid_start;
    if (editor->buffer_id == bid_start) output_bold();
    for (int i = 0; i < box.width && i < editor->tab_bar->size; ++i) {
        char c = editor->tab_bar->buffer[i + editor->tab_scroll];
        if (c == '\n') {
            if (bid == editor->buffer_id) output_normal();
            output_altchar_on();
            output_char(ALTCHAR_VLINE);
            output_altchar_off();
            bid++;
            if (bid == editor->buffer_id) output_bold();
        } else {
            output_char(c);
        }
    }

}


// //
// // Altbuffer Line.
// //
//
// static
// void draw_alt_buffer (Editor* editor, Box box) {
//     const char* prompt;
//     switch (editor->alt_mode) {
//         case ALT_OPEN:
//             prompt = " (Open) ";
//             break;
//         case ALT_SAVE:
//             prompt = " (SAVE) ";
//             break;
//         default:
//             return;
//     }
//
//     int prompt_len = strlen(prompt);
//
//     CharBuffer* contents = charbuffer_create();
//     buffer_get_contents(editor->alt_buffer, contents);
//
//     char buf[box.width + 1];
//     snprintf("%s%s", box.width + 1, prompt, contents->buffer);
//     output_cup(box.y, box.x);
//     output_str(buf);
//     output_cup(box.y, box.x + prompt_len + editor->alt_buffer->cursor.col);
//
// }
