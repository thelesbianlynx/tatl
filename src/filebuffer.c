#include "filebuffer.h"

#include "charbuffer.h"
#include "intbuffer.h"
#include "rope.h"
#include "textbuffer.h"
#include "textview.h"
#include "codepoint.h"
#include "output.h"


FileBuffer* filebuffer_create () {
    FileBuffer* fb = malloc(sizeof(FileBuffer));
    fb->buffer = textbuffer_create(rope_create(NULL));
    fb->view = textview_create(fb->buffer);
    fb->title = charbuffer_create();
    fb->longpath = charbuffer_create();
    fb->shortpath = charbuffer_create();

    charbuffer_astr(fb->title, "Untitled");

    return fb;
}

void filebuffer_destroy (FileBuffer* fb) {
    textbuffer_destroy(fb->buffer);
    textview_destroy(fb->view);
    charbuffer_destroy(fb->title);
    charbuffer_destroy(fb->longpath);
    charbuffer_destroy(fb->shortpath);
    free(fb);
}


static
uint32_t last_slash(const char* path) {
    int32_t i = 0;
    int32_t r = 0;
    while (path[i] != '\0') {
        if (path[i] == '/') r = i;
        i++;
    }
    return r;
}

static
char* set_path (FileBuffer* fb, const char* path) {
    char* rpath = realpath(path, NULL);

    // Long Path.
    charbuffer_clear(fb->longpath);
    charbuffer_astr(fb->longpath, rpath);

    // Short Path.
    //  - Will eventually be an abridged version of long path for display purposes.
    //  - Same for now.
    charbuffer_clear(fb->shortpath);
    charbuffer_astr(fb->shortpath, rpath);

    // Title.
    //  - Just the filename.
    charbuffer_clear(fb->title);
    charbuffer_astr(fb->title, rpath + last_slash(rpath));

    free(rpath);
    return fb->longpath->buffer;
}

void filebuffer_read (FileBuffer* fb, const char* path) {
    char* filepath = set_path(fb, path);

    CharBuffer* chars = charbuffer_create();

    FILE* f = fopen(filepath, "r");
    charbuffer_read(chars, f);
    fclose(f);

    IntBuffer* chars32 = intbuffer_create();
    intbuffer_put_text(chars32, 0, chars);
    charbuffer_destroy(chars);

    if (chars32->data[chars32->size - 1] == '\n') {
        chars32->size--; // Remove Ending Newline.
    }

    Rope* text = rope_create(chars32);
    intbuffer_destroy(chars32);

    textbuffer_destroy(fb->buffer);
    fb->buffer = textbuffer_create(text);

    textview_destroy(fb->view);
    fb->view = textview_create(fb->buffer);
}

static
bool rope_write (uint32_t i, uint32_t ch, void* data) {
    FILE* file = data;
    char buf[5] = {0};
    codepoint_to_chars(buf, ch);
    fputs(buf, file);

    return true;
}

void filebuffer_write (FileBuffer* fb, const char* path) {
    char* filepath = set_path(fb, path);

    FILE* f = fopen(filepath, "w");
    rope_foreach(fb->buffer->text, rope_write, f);
    fputc('\n', f); // Put Back Ending Newline.
    fclose(f);

    fb->buffer->text_dmg = false;
}


void filebuffer_draw (FileBuffer* fb, Box* window, MouseEvent* mev) {
    // Make sure window is big enough.
    //  - This needs to not be an assert at some point.
    assert(window->height >= 3);

    // Draw Status.
    {
        uint32_t line = window->y + window->height - 1;
        uint32_t width = window->width;

        char buf[width+1];

        // First line: 'short' path name.
        output_cup(line, 0);
        //output_reverse();
        snprintf(buf, width + 1, " %-*s ", width - 2, fb->shortpath->size > 0 ? fb->shortpath->buffer : "[Untitled]");
        output_str(buf);
        output_normal();
    }

    // Draw Text.
    {
        Box textwin = { window->x, window->y, window->width, window->height - 1 };
        textview_draw(fb->view, &textwin, mev);
    }
}
