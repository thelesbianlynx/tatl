#include "editor.h"

#include "array.h"
#include "charbuffer.h"
#include "rope.h"
#include "textbuffer.h"
#include "textview.h"
#include "textaction.h"
#include "filebuffer.h"
#include "input.h"
#include "output.h"


enum {
    ALT_NONE = 0,
    ALT_OPEN,
    ALT_SAVE,
    ALT_SEARCH,
    ALT_FIND,
    ALT_REPLACE,
};

void editor_init (Editor* editor, Array* filenames) {
    editor->buffers = array_create();
    editor->current_buffer = 0;

    editor->altmode = 0;
    editor->altbuffer = textbuffer_create(rope_create(NULL));
    editor->altview = textview_create(editor->altbuffer);
    editor->altview->linenos = false;

    editor->clipboard = array_create();
    editor->dir = charbuffer_create();

    char* cwd = getcwd(NULL, 0);
    assert(*cwd == '/');
    charbuffer_astr(editor->dir, cwd);
    free(cwd);

    if (filenames->size == 0) {
        FileBuffer* fb = filebuffer_create();
        array_add(editor->buffers, fb);
    } else {
        for (int i = 0; i < filenames->size; i++) {
            FileBuffer* fb = filebuffer_create();
            filebuffer_read(fb, filenames->data[i]);
            array_add(editor->buffers, fb);
        }
    }
}

void editor_fini (Editor* editor) {
    for (int i = 0; i < editor->buffers->size; i++) {
        filebuffer_destroy(editor->buffers->data[i]);
    }
    for (int i = 0; i < editor->clipboard->size; i++) {
        rope_destroy(editor->clipboard->data[i]);
    }
    array_destroy(editor->buffers);
    array_destroy(editor->clipboard);
    charbuffer_destroy(editor->dir);
}


static
FileBuffer* get_buffer(Editor* editor) {
    editor->current_buffer = MOD(editor->current_buffer, editor->buffers->size);
    return editor->buffers->data[editor->current_buffer];
}

static
bool altbuffer_event (Editor* editor, InputEvent* event) {
    ON_KEY(event) {
        KEY_CTRL(event) {
            CTRL('Q') {
                return false;
            }

            default: {
                textaction(event, editor->altbuffer, 1, editor->clipboard);
                break;
            }
        } break;

        KEY_ALT(event) {

            default: {
                textaction(event, editor->altbuffer, 1, editor->clipboard);
                break;
            }
        } break;

        KEY_ESC {
            editor->altmode = 0;
            break;
        }

        KEY_ENTER {
            switch (editor->altmode) {
                case ALT_OPEN: {
                    CharBuffer* filename = charbuffer_create();
                    textbuffer_get_contents(editor->altbuffer, filename);

                    FileBuffer* fb = get_buffer(editor);
                    if (rope_len(fb->buffer->text) > 0 || fb->longpath->size > 0) {
                        fb = filebuffer_create();
                        array_add(editor->buffers, fb);
                        editor->current_buffer = editor->buffers->size - 1;
                    }

                    filebuffer_read(fb, filename->buffer);

                    charbuffer_destroy(filename);
                    break;
                }
                case ALT_SAVE: {
                    CharBuffer* filename = charbuffer_create();
                    textbuffer_get_contents(editor->altbuffer, filename);

                    FileBuffer* fb = get_buffer(editor);
                    filebuffer_write(fb, filename->buffer);

                    charbuffer_destroy(filename);
                    break;
                }
            }

            editor->altmode = 0;
            break;
        }

        default: {
            textaction(event, editor->altbuffer, 1, editor->clipboard);
            break;
        }
    }

    return true;
}

bool editor_event (Editor* editor, InputEvent* event) {
    if (editor->altmode) {
        return altbuffer_event(editor, event);
    }

    FileBuffer* fb = get_buffer(editor);

    ON_KEY(event) {
        KEY_CTRL(event) {
            CTRL('Q') {
                return false;
            }

            CTRL('W') {
                if (editor->buffers->size <= 1) {
                    return false;
                }

                FileBuffer* rm_fb = array_remove(editor->buffers, editor->current_buffer);
                if (editor->current_buffer >= editor->buffers->size) {
                    editor->current_buffer--;
                }

                filebuffer_destroy(rm_fb);
                break;
            }

            CTRL('N') {
                FileBuffer* new_fb = filebuffer_create();
                array_add(editor->buffers, new_fb);
                editor->current_buffer = editor->buffers->size - 1;
                break;
            }

            CTRL('O') {
                textbuffer_set_contents(editor->altbuffer, NULL);
                editor->altmode = ALT_OPEN;
                break;
            }

            CTRL('S') {
                textbuffer_set_contents(editor->altbuffer, fb->longpath);
                editor->altmode = ALT_SAVE;
                break;
            }

            CTRL('P') {
                editor->altmode = ALT_SEARCH;
                break;
            }

            default: {
                textaction(event, fb->buffer, 1, editor->clipboard);
                break;
            }
        } break;

        KEY_ALT(event) {

            ALT('t') {
                editor->current_buffer++;
                break;
            }

            ALT('T') {
                editor->current_buffer--;
                break;
            }

            default: {
                textaction(event, fb->buffer, 1, editor->clipboard);
                break;
            }
        } break;

        default: {
            textaction(event, fb->buffer, 1, editor->clipboard);
            break;
        }
    }

    if (editor->buffers->size == 0) {
        return false;
    }

    return true;
}

static
const char* prompt_string (Editor* editor) {
    switch(editor->altmode) {
        case ALT_OPEN:
            return " (OPEN) ";
        case ALT_SAVE:
            return " (SAVE) ";
        default:
            return "";
    }
}

void editor_draw (Editor* editor, Box* window, MouseEvent* m_event) {
    FileBuffer* fb = get_buffer(editor);

    int header_size = 1;
    int altbuffer_size = editor->altmode == 0 ? 0 : 1;

    // Header & Tab-Bar.
    {
        char buf[64];
        snprintf(buf, 64, "[Buffers] %d / %d", editor->current_buffer + 1, editor->buffers->size);
        output_cup(window->y, window->x);
        output_str(buf);
    }

    // Current Buffer.
    {
        Box fb_window = {
            window->x, window->y + header_size,
            window->width, window->height - header_size - altbuffer_size };
        filebuffer_draw(fb, &fb_window, m_event);
    }

    // Altbuffer.
    {
        const char* prompt = prompt_string(editor);
        int prompt_ln = strlen(prompt);
        if (prompt_ln > 0) {
            output_bold();
            output_cup(window->y + window->height - altbuffer_size, window->x);
            output_str(prompt);
            output_normal();

            Box alt_window = {
                window->x + prompt_ln, window->y + window->height - altbuffer_size,
                window->width - prompt_ln, altbuffer_size };
            textview_draw(editor->altview, &alt_window, m_event);

        }
    }
}
