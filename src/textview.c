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

typedef struct putchar_data PutCharData;
struct putchar_data {
    uint32_t start, end, line_start;
    uint8_t curr;
    uint8_t* style;
};

static
bool textview_putchar (uint32_t i, uint32_t ch, void* data) {
    PutCharData* p = data;
    if (i < p->end) {
        uint32_t x = i - p->start;
        uint8_t s = p->style[x + p->line_start];

        // Set style.
        if (s != p->curr) {
            output_normal();
            p->curr = s;

            if (s & 1) {
                output_setbg(12);
            }
            if (s & 2) {
                output_reverse();
            }
        }

        // Put character.
        if (ch >= ' ') {
            output_uchar(ch);
        } else if (ch == '\t') {
            // ...
        } else {
            output_uchar(' ');
        }

        // Continue.
        return true;
    }

    return false;
}

void textview_draw (TextView* view, Box* window, MouseEvent* mstate) {
    TextBuffer* buffer = view->buffer;

    // Width of line numbers.
    int32_t ln_width = (int) log10(rope_lines(buffer->text) + 1) + 3;

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

    // Text start and end indices.
    uint32_t text_start = istart;
    uint32_t text_end = rope_point_to_index(buffer->text, (Point) {view->scroll_line + text_height, 0});

    // Text style.
    uint8_t style[text_end - text_start] = {};

    // Primary Selection Cursor position.
    Point primary = {};

    // Fill out style from selections.
    for (int i = 0; i < buffer->selections->size; i++) {
        Selection* sel = buffer->selections->data[i];
        int sel_start = MIN(sel->cursor, sel->anchor);
        int sel_end = MAX(sel->cursor, sel->anchor);

        while (sel_start < sel_end) {
            if (text_start <= sel_start && sel_start < text_end) {
                style[sel_start - text_start] |= 1;
            }

            sel_start++;
        }
        if (buffer->selections->size > 1) {
            if (text_start <= sel->cursor && sel->cursor < text_end) {
                style[sel->cursor - text_start] |= 2;
            }
        }
        if (sel->primary) {
            primary = rope_index_to_point(buffer->text, sel->cursor);
        }
    }

    for (int i = 0; i < text_height; i++) {
        // Update end position with next line start.
        iend = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i + 1, 0});

        // End of buffer.
        if (istart == iend) {
            break;
        }

        // Line number.
        output_cup(window->y + i, window->x);
        output_normal();
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

        // Style struct to pass to putchar.
        PutCharData p = {
            .start = text_start,
            .end = iend,
            .line_start = 0,// i*text_width,
            .curr = 0,
            .style = style,
        };

        // Line content.
        output_normal();
        rope_foreach_substr(buffer->text, istart, iend, textview_putchar, &p); //(void*) (intptr_t) (istart + text_width));

        // Update start of next line.
        istart = iend;
    }

    // cursor pos:
    char buf[64];
    Point c = primary;
    snprintf(buf, 64, "%d:%d", c.row, c.col);
    output_cup(window->y + window->height - 1, window->x + 1);
    output_normal();
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
