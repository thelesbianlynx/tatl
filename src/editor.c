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
    editor->tab_scroll = 0;

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
                editor->tab_scroll_dmg = true;
                break;
            }

            CTRL('N') {
                FileBuffer* new_fb = filebuffer_create();
                array_add(editor->buffers, new_fb);
                editor->current_buffer = editor->buffers->size - 1;
                editor->tab_scroll_dmg = true;
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
                editor->tab_scroll_dmg = true;
                break;
            }

            ALT('T') {
                editor->current_buffer--;
                editor->tab_scroll_dmg = true;
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

static void build_tab_bar (Editor* editor, uint32_t width, CharBuffer* tab_bar);
static void draw_tab_bar (Editor* editor, Box* window, CharBuffer* tab_bar, MouseEvent* mev);

void editor_draw (Editor* editor, Box* window, MouseEvent* m_event) {
    int header_size = 3;
    int altbuffer_size =  1; // editor->altmode == 0 ? 0 : 1;

    // Header & Tab-Bar.
    {
        int32_t width = window->width;

        char buf[width+1], left[width+1];
        snprintf(left, width+1, "[Buffers] %d / %d", editor->current_buffer + 1, editor->buffers->size);
        snprintf(buf, width+1, " %s %*s ", left, width - 3 - (int) strlen(left), editor->dir->buffer);
        output_cup(window->y, window->x);
        output_setfg(13);
        output_reverse();
        output_str(buf);
        output_normal();

        CharBuffer* tab_bar = charbuffer_create();
        Box tab_bar_window = { window->x, window->y + 1, width, 2 };

        build_tab_bar(editor, width, tab_bar);
        draw_tab_bar(editor, &tab_bar_window, tab_bar, m_event);

        charbuffer_destroy(tab_bar);
    }

    // Current Buffer.
    {
        FileBuffer* fb = get_buffer(editor);
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


//
// Tab Bar
//
// - This code was lifted from before the rewrite.
// - It may not be of the best quality...

static
void draw_tab_bar (Editor* editor, Box* window, CharBuffer* tab_bar, MouseEvent* mev) {
    assert(tab_bar->size >= window->width);

    int bid_start = 0;
    for (int i = 0; i < editor->tab_scroll; ++i) {
        if (tab_bar->buffer[i] == '\n') bid_start++;
    }

    // Mouse Input.
    if (mev != NULL && editor->altmode == 0) {
        int32_t mx = mev->x, my = mev->y;
        if (mx > window->x && my > window->y) {
            mx -= window->x;
            my -= window->y;
            if (mx <= window->width && my <= window->height) {
                if (mev->button == 0) {
                    // Press.
                    int bid = bid_start;
                    for (int i = 0; i < mx; ++i) {
                        if (tab_bar->buffer[i + editor->tab_scroll] == '\n') bid++;
                    }
                    editor->current_buffer = MOD(bid, editor->buffers->size);
                    editor->tab_scroll_dmg = true;
                } else if (mev->button == 32) {
                    // Drag.

                } else if (mev->button == 64) {
                    // Scroll Up.
                    editor->tab_scroll -= 2;
                    if (editor->tab_scroll >= 0 && tab_bar->buffer[editor->tab_scroll] == '\n') bid_start--;
                } else if (mev->button == 65) {
                    // Scroll Down.
                    if (tab_bar->buffer[editor->tab_scroll] == '\n') bid_start++;
                    editor->tab_scroll += 2;
                }
            }
        }
    }

    // Scroll damage.
    if (editor->tab_scroll_dmg) {
        if (tab_bar->size > window->width) {
            int current = MOD(editor->current_buffer, editor->buffers->size);
            if (current <= bid_start) {
                int bid = 0;
                for (int i = 0; i < window->width && i < tab_bar->size; ++i) {
                    if (tab_bar->buffer[i] == '\n') bid++;
                    if (bid == current) {
                        editor->tab_scroll = i;
                        break;
                    }
                }
                bid_start = current;
            } else {
                int bid = bid_start;
                for (int i = editor->tab_scroll; i < tab_bar->size; ++i) {
                    if (tab_bar->buffer[i] == '\n') {
                        bid++;
                        if (bid == current) {
                            int size = ((FileBuffer*) editor->buffers->data[current])->title->size;
                            while (i + size - editor->tab_scroll + 3 > window->width) {
                                if (tab_bar->buffer[editor->tab_scroll] == '\n') bid_start++;
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
    if (editor->tab_scroll > tab_bar->size - window->width)  {
        editor->tab_scroll = tab_bar->size - window->width;
    }

    //
    // Main Line.
    output_cup(window->y, window->x);
    int bid = bid_start;
    if (editor->current_buffer == bid_start) output_bold();
    if (((FileBuffer*)editor->buffers->data[0])->buffer->text_dmg) output_italic();
    for (int i = 0; i < window->width && i < tab_bar->size; ++i) {
        char c = tab_bar->buffer[i + editor->tab_scroll];
        if (c == '\n') {
            output_normal();
            output_setfg(13);
            output_uchar(0x2503);
            output_normal();
            bid++;
            if (bid == editor->current_buffer) output_bold();
            if (bid < editor->buffers->size) {
                FileBuffer* fb = editor->buffers->data[bid];
                if (fb->buffer->text_dmg) output_italic();
            }
        } else {
            output_char(c);
        }
    }
    output_normal();

    // Base Line.
    output_cup(window->y + 1, window->x);
    output_setfg(13);
    bid = bid_start;
    if (editor->current_buffer == bid_start)
        output_setfg(12);
    for (int i = 0; i < window->width && i < tab_bar->size; ++i) {
        char c = tab_bar->buffer[i + editor->tab_scroll];
        if (c == '\n') {
            bid++;
            if (bid == editor->current_buffer) {
                output_uchar(0x2594); //0x2580);
                output_setfg(12);
            } else {
                output_setfg(13);
                output_uchar(0x2594); //0x2580);
            }
        } else {
            output_uchar(0x2594); //0x2580);
        }
    }
    output_normal();
}


static
void build_tab_bar (Editor* editor, uint32_t width, CharBuffer* tab_bar) {
    charbuffer_clear(tab_bar);

    int sum = 0;
    for (int i = 0; i < editor->buffers->size; ++i) {
        FileBuffer* fb = editor->buffers->data[i];
        sum += fb->title->size + 3;
    }

    if (sum > width) {
        for (int i = 0; i < editor->buffers->size; ++i) {
            FileBuffer* fb = editor->buffers->data[i];
            charbuffer_achar(tab_bar, ' ');
            charbuffer_achars(tab_bar, fb->title);
            charbuffer_achar(tab_bar, ' ');
            if (i < editor->buffers->size - 1) {
                charbuffer_achar(tab_bar, '\n');
            }
        }
    } else {
        int rem = 64 * (width - sum);
        int each = rem / editor->buffers->size;

        for (int i = 0; i < editor->buffers->size; ++i) {
            FileBuffer* fb = editor->buffers->data[i];
            charbuffer_achar(tab_bar, ' ');
            charbuffer_achars(tab_bar, fb->title);
            if (i < editor->buffers->size - 1) {
                for (int x = 0; x < each; x += 64) {
                    if (rem > 0) {
                        charbuffer_achar(tab_bar, ' ');
                        rem -= 64;
                    }
                }
                charbuffer_astr(tab_bar, " \n");
            } else {
                while (tab_bar->size < width) {
                    charbuffer_achar(tab_bar, ' ');
                }
            }
        }
    }
}
