#include "editor.h"

#include "array.h"
#include "charbuffer.h"
#include "rope.h"
#include "textbuffer.h"
#include "textview.h"
#include "textaction.h"
#include "filebuffer.h"
#include "search.h"
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
    editor->tab_scroll_dmg = false;

    editor->search_files = array_create();
    editor->search_selection = 0;
    editor->search_scroll = 0;
    editor->search_scroll_dmg = false;

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
    array_destroy(editor->search_files);
    charbuffer_destroy(editor->dir);
}


static
FileBuffer* get_buffer(Editor* editor) {
    editor->current_buffer = MOD(editor->current_buffer, editor->buffers->size);
    return editor->buffers->data[editor->current_buffer];
}


//
// Update Editor State.
//

static bool altbuffer_event (Editor* editor, InputEvent* event);
static bool search_event (Editor* editor, InputEvent* event);

bool editor_event (Editor* editor, InputEvent* event) {
    if (editor->altmode) {
        return altbuffer_event(editor, event);
    }

    FileBuffer* fb = get_buffer(editor);

    ON_KEY(event) {
        KEY_CTRL('Q') {
            return false;
        }

        KEY_CTRL('W') {
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

        KEY_CTRL('N') {
            FileBuffer* new_fb = filebuffer_create();
            array_add(editor->buffers, new_fb);
            editor->current_buffer = editor->buffers->size - 1;
            editor->tab_scroll_dmg = true;
            break;
        }

        KEY_CTRL('O') {
            textbuffer_set_contents(editor->altbuffer, NULL);
            editor->altmode = ALT_OPEN;
            break;
        }

        KEY_CTRL('S') {
            textbuffer_set_contents(editor->altbuffer, fb->longpath);
            editor->altmode = ALT_SAVE;
            break;
        }

        // KEY_CTRL('P') {
        KEY_ALT_ENTER {
            textbuffer_set_contents(editor->altbuffer, NULL);
            search_load_files(editor->search_files, editor->dir->buffer);
            editor->altmode = ALT_SEARCH;
            editor->search_selection = 0;
            editor->search_scroll = 0;
            break;
        }

        KEY_ALT('t') {
            editor->current_buffer++;
            editor->tab_scroll_dmg = true;
            break;
        }

        KEY_ALT('T') {
            editor->current_buffer--;
            editor->tab_scroll_dmg = true;
            break;
        }

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

// -- Alt-Mode Event Handler -- //

static
bool altbuffer_event (Editor* editor, InputEvent* event) {
    if (editor->altmode == ALT_SEARCH) {
        return search_event(editor, event);
    }

    ON_KEY(event) {
        KEY_CTRL('Q') {
            return false;
        }

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

// -- Search-Mode Event Handler -- //

static
bool search_event (Editor* editor, InputEvent* event) {
    ON_KEY(event) {
        KEY_CTRL('Q') {
            search_unload_files(editor->search_files);
            return false;
        }

        KEY_ESC {
            search_unload_files(editor->search_files);
            editor->altmode = 0;
            break;
        }

        KEY_ENTER {
            FileEntry* entry = editor->search_files->data[editor->search_selection];

            // Check if file is already open.
            for (int i = 0; i < editor->buffers->size; i++) {
                FileBuffer* fb = editor->buffers->data[i];
                if (strcmp(entry->path->buffer, fb->longpath->buffer) == 0) {
                    editor->current_buffer = i;
                    search_unload_files(editor->search_files);
                    editor->altmode = 0;
                    return false;
                }
            }

            // Otherwise open new file.
            FileBuffer* fb = get_buffer(editor);
            if (rope_len(fb->buffer->text) > 0 || fb->longpath->size > 0) {
                fb = filebuffer_create();
                array_add(editor->buffers, fb);
                editor->current_buffer = editor->buffers->size - 1;
            }

            filebuffer_read(fb, entry->path->buffer);

            search_unload_files(editor->search_files);
            editor->altmode = 0;
            break;
        }

        KEY_UP {
            editor->search_selection = MOD(editor->search_selection - 1, editor->search_files->size);
            editor->search_scroll_dmg = true;
            break;
        }

        KEY_DOWN {
            editor->search_selection = MOD(editor->search_selection + 1, editor->search_files->size);
            editor->search_scroll_dmg = true;
            break;
        }

        // Don't Pass these keys to textaction.
        KEY_SHIFT_UP { break; };
        KEY_SHIFT_DOWN { break; };
        KEY_CTRL_UP { break; };
        KEY_CTRL_DOWN { break; };
        KEY_SHIFT_CTRL_UP { break; };
        KEY_SHIFT_CTRL_DOWN { break; };

        default: {
            textaction(event, editor->altbuffer, 1, editor->clipboard);
            CharBuffer* query = charbuffer_create();
            textbuffer_get_contents(editor->altbuffer, query);
            search_rank_files(editor->search_files, query->buffer);
            charbuffer_destroy(query);
            editor->search_selection = 0;
            editor->search_scroll = 0;
            break;
        }
    }

    return true;
}


//
// Draw Editor.
//

static void build_tab_bar (Editor* editor, uint32_t width, CharBuffer* tab_bar);
static void draw_tab_bar (Editor* editor, Box* window, CharBuffer* tab_bar, MouseEvent* mev);
static void draw_search (Editor* editor, Box* window, MouseEvent* mev);

static
const char* prompt_string (Editor* editor) {
    switch(editor->altmode) {
        case ALT_OPEN:
            return " (OPEN) ";
        case ALT_SAVE:
            return " (SAVE) ";
        case ALT_SEARCH:
            return " (SEARCH) ";
        default:
            return "";
    }
}

void editor_draw (Editor* editor, Box* window, MouseEvent* m_event) {
    int header_size = 3;
    int altbuffer_size =  1; // editor->altmode == 0 ? 0 : 1;
    int search_window_size = editor->altmode == ALT_SEARCH ? (window->height - header_size)/2 : 0;

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
            window->width, window->height - header_size - altbuffer_size - search_window_size };
        filebuffer_draw(fb, &fb_window, m_event);
    }

    // Altbuffer & Search Window.
    {
        const char* prompt = prompt_string(editor);
        int prompt_ln = strlen(prompt);
        if (prompt_ln > 0) {
            output_bold();
            output_cup(window->y + window->height - altbuffer_size - search_window_size, window->x);
            output_str(prompt);
            output_normal();

            Box alt_window = {
                window->x + prompt_ln, window->y + window->height - altbuffer_size - search_window_size,
                window->width - prompt_ln, altbuffer_size + search_window_size };
            textview_draw(editor->altview, &alt_window, m_event);

            if (search_window_size > 0) {
                Box search_window = {
                    window->x + prompt_ln - 1, window->y + window->height - search_window_size,
                    window->width - prompt_ln, search_window_size };
                draw_search(editor, &search_window, m_event);
            }
        }
    }
}


// -- Tab Bar -- //

//
// This code was lifted from before the rewrite. It may not be of the best quality...

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

// -- Search Window -- //

static
void draw_search (Editor* editor, Box* window, MouseEvent* mev) {
    // Mouse Input.
    if (mev != NULL) {

    }

    // Scroll damage.
    if (editor->search_scroll_dmg) {
        if (editor->search_selection < editor->search_scroll) {
            editor->search_scroll = editor->search_selection;
        }
        if (editor->search_selection > editor->search_scroll + window->height - 1) {
            editor->search_scroll = MAX(0, editor->search_selection - window->height + 1);
        }

        editor->search_scroll_dmg = false;
    }

    for (int i = 0; i < window->height; i++) {
        int32_t n = i + editor->search_scroll;
        if (n >= editor->search_files->size) break;

        FileEntry* file = editor->search_files->data[n];

        char buf[window->width + 1];
        snprintf(buf, window->width + 1 ," %s ", file->path->buffer + file->prefix + 1);

        output_cup(window->y + i, window->x);
        if (n == editor->search_selection) {
            output_setbg(12);
            output_str(buf);
            output_normal();
        } else {
            output_str(buf);
        }
    }
}
