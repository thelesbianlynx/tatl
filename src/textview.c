#include "textview.h"

#include "array.h"
#include "input.h"
#include "output.h"
#include "rope.h"
#include "textbuffer.h"

TextView* textview_create (TextBuffer* buffer) {
    TextView* view = malloc(sizeof(TextView));
    view->buffer = buffer;
    view->scroll_line = 0;
    view->scroll_col = 0;

    return view;
}

void textview_destroy (TextView* view) {
    free(view);
}


static
bool textview_putchar (uint32_t i, uint32_t ch, void* data) {
    if (i < (intptr_t) data) {
        if (ch >= ' ') {
            output_uchar(ch);
        } else if (ch == '\t') {
            // ...
        }
        return true;
    }

    return false;
}

void textview_draw (TextView* view, Box* window, MouseEvent* mstate) {
    TextBuffer* buffer = view->buffer;

    // Width of line numbers.
    int32_t ln_width = (int) log10(rope_lines(buffer->text)) + 3;

    // Alt Buffer Prompt width.
    // if (buffer->altmode) {
    //     ln_width = buffer->title->size;
    // }

    // Mouse Input.
    if (mstate != NULL) {

    }

    // Width of Text area.
    int32_t text_width = window->width - ln_width;
    // Height of Text area.
    int32_t text_height = window->height - 1;

    // Update Scroll.
    if (buffer->cursor_dmg) {
        // Scroll Line.
        if (buffer->altmode) {
            view->scroll_line = 0;
        } else {
            int32_t top = rope_index_to_point(buffer->text, ((Selection*) buffer->selections->data[0])->cursor).row;
            //int32_t bot = rope_index_to_point(buffer->text, ((Selection*) array_peek(buffer->selections))->cursor).row;

            if (top < view->scroll_line) {
                view->scroll_line = top;
            } else if (top > view->scroll_line + text_height - 2) {
                view->scroll_line = top - text_height + 2;
            }
        }

        // Scroll Column
        //  ...

        // Unset Damage Flag.
        buffer->cursor_dmg = false;
    }


    // Line start and end indices.
    uint32_t istart = rope_point_to_index(buffer->text, (Point) {view->scroll_line, 0});
    uint32_t iend;

    for (int i = 0; i < text_height; i++) {
        // Update end position with next line start.
        iend = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i + 1, 0});

        // End of buffer.
        if (istart == iend) {
            break;
        }

        // Line number.
        output_cup(window->y + i, window->x);
        output_bold();
        if (!buffer->altmode) {
            char ln_buf[ln_width + 1];
            snprintf(ln_buf, ln_width + 1, "%*d ", ln_width - 1 , view->scroll_line + i + 1);
            output_str(ln_buf);
        } else {
            // Alt mode prompt instead of line number.
            // for (int j = 0; j < buffer->title->size; ++j) {
                // output_char(buffer->title->buffer[j]);
            // }
        }

        // Line content.
        output_normal();
        rope_foreach_substr(buffer->text, istart, iend, textview_putchar, (void*) (intptr_t) (istart + text_width));

        // Update start of next line.
        istart = iend;
    }

    // cursor pos:
    output_cup(window->y + window->height - 1, window->x + 1);
    char buf[64];
    Point c = rope_index_to_point(buffer->text, ((Selection*)array_peek(buffer->selections))->cursor);
    snprintf(buf, 64, "%d:%d %d", c.row, c.col, view->scroll_line);
    output_str(buf);

    // Cursor.
    if (c.row >= view->scroll_line && c.row < view->scroll_line + text_height &&
        c.col >= view->scroll_col && c.col < view->scroll_col + text_width) {
            output_cvvis();
            output_cup(c.row + window->y - view->scroll_line, c.col + window->x + ln_width - view->scroll_col);
    } else {
        output_civis();
    }

}
