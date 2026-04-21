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
    view->linenos = true;

    return view;
}

void textview_destroy (TextView* view) {
    free(view);
}


//
// Borrowed From TextBuffer.
static inline
uint32_t head (Selection* sel) {
    return MIN(sel->cursor, sel->anchor);
}
static inline
uint32_t tail (Selection* sel) {
    return MAX(sel->cursor, sel->anchor);
}


extern bool cursor_blink;
struct draw_char_data {
    int32_t* chars;
    uint32_t i_chars;

    int32_t* styles;
    uint32_t i_styles;

    uint32_t scroll_col;
    uint32_t col_width;
    uint32_t i_col;

    Array* selections;
    uint32_t tab_width;
};

static
bool char_style (uint32_t i, uint32_t ch, void* d) {
    struct draw_char_data* data = d;
    if (data->i_chars >= data->col_width) return true;

    // Cursor Style.
    int32_t style = 0;
    for (int x = 0; x < data->selections->size; x++) {
        Selection* sel = data->selections->data[x];
        if (head(sel) <= i && i < tail(sel)) style |= 1;
        if (i == sel->cursor /*&& data->selections->size > 1*/ && !cursor_blink) style |= 2;
    }

    // Colorizer Logic.
    //  ....

    // Emit Character and Style.

    // Special case: Hard Tab.
    if (ch == '\t') {
        int t = data->tab_width - (data->i_col % data->tab_width);
        for (int x = 0; x < t; x++) {
            if (data->i_chars >= data->col_width) return true;
            if (data->i_col >= data->scroll_col){
                data->chars[data->i_chars] = ' ';
                data->styles[data->i_chars] |= style;
                data->i_chars++;
            }
            data->i_col++;
        }
    }
    // Special case: Control Character.
    else if (ch < 32) {
        if (data->i_col >= data->scroll_col){
            data->chars[data->i_chars] = ' ';
            data->styles[data->i_chars] |= style;
            data->i_chars++;
        }
        data->i_col++;
    }
    // General case: Regular Character.
    else {
        if (data->i_col >= data->scroll_col){
            data->chars[data->i_chars] = ch;
            data->styles[data->i_chars] |= style;
            data->i_chars++;
        }
        data->i_col++;
    }

    return true;
}


void textview_draw (TextView* view, Box* window, MouseEvent* mstate) {
    TextBuffer* buffer = view->buffer;

    // Width of line numbers.
    int32_t ln_width = view->linenos ? (int32_t) log10(rope_lines(buffer->text) + 1) + 3 : 0;

    // Width of Text area.
    int32_t text_width = window->width - ln_width;
    // Height of Text area.
    int32_t text_height = window->height;

    // Number of lines in buffer (at least 1).
    int32_t lines = rope_lines(buffer->text) + 1;

    // Mouse Input.
    if (mstate != NULL) {
        int32_t mx = mstate->x, my = mstate->y;
        if (mx > window->x && my > window->y) {
            mx -= window->x;
            my -= window->y;
            if (mx <= window->width && my <= window->height) {
                if (mstate->button == 0) {
                    // Press.
                    textbuffer_cursor_goto(buffer, my + view->scroll_line - 1, mx + view->scroll_col - ln_width - 1, false);
                } else if (mstate->button == 32) {
                    // Drag.
                    textbuffer_cursor_goto(buffer, my + view->scroll_line - 1, mx + view->scroll_col - ln_width - 1, true);
                } else if (mstate->button == 64) {
                    // Scroll Up.
                    view->scroll_line -= 2;
                } else if (mstate->button == 65) {
                    // Scroll Down.
                    view->scroll_line += 2;
                }
            }
        }
    }

    // Update Scroll.
    if (buffer->cursor_dmg) {
        // Scroll Line.
        int32_t top = rope_index_to_point(buffer->text, ((Selection*) buffer->selections->data[0])->cursor).row;
        //int32_t bot = rope_index_to_point(buffer->text, ((Selection*) array_peek(buffer->selections))->cursor).row;
        if (text_height == 1) {
            view->scroll_line = top;
        } else if (top < view->scroll_line) {
            view->scroll_line = top;
        } else if (top > view->scroll_line + text_height - 2) {
            view->scroll_line = MAX(0, top - text_height + 2);
        }

        // Scroll Column
        //  ...

        // Unset Damage Flag.
        buffer->cursor_dmg = false;
    }

    // Clamp Scroll.
    if (view->scroll_line >= lines) view->scroll_line = lines - 1;
    if (view->scroll_line < 0) view->scroll_line = 0;

    // Buffers for char and style data.
    int32_t chars[text_width];
    int32_t styles[text_width];

    for (int i = 0; i < text_height; i++) {

        // End of Buffer.
        if (view->scroll_line + i >= lines) {
            break;
        }

        // Start and end of line.
        int32_t start = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i, 0});
        int32_t end = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i, INT_MAX});

        // Line number.
        output_cup(window->y + i, window->x);
        output_normal();
        if (view->linenos) {
            output_bold();
            char ln_buf[ln_width + 1];
            snprintf(ln_buf, ln_width + 1, "%*d ", ln_width - 1 , view->scroll_line + i + 1);
            output_str(ln_buf);
            output_normal();
        }

        memset(chars, 0, sizeof chars);
        memset(styles, 0, sizeof styles);
        struct draw_char_data data = {
            .scroll_col = view->scroll_col,
            .col_width = text_width,
            .chars = chars,
            .styles = styles,
            .selections = buffer->selections,
            .tab_width = buffer->tab_width,
        };

        // Get Line Content and Style.
        rope_foreach_substr(buffer->text, start, end, char_style, &data);
        char_style(end, '\n', &data);

        //  Write Line Content.
        int32_t current_style = 0;
        for (int x = 0; x < text_width; x++) {
            int32_t ch = chars[x];
            int32_t style = styles[x];

            // End of line before end of screen.
            if (ch == 0) break;

            // Set style.
            if (style != current_style) {
                output_normal();
                current_style= style;

                if (style & 1) {
                    output_setbg(12);
                }
                if (style & 2) {
                    output_underline();
                }
            }
            // Put character.
            assert(ch >= 32 && "Invalid Ouput Character");
            output_uchar(ch);
        }
    }

    // cursor pos:
    // char buf[64];
    // Point c = primary;
    // snprintf(buf, 64, "%d:%d (%d, %d) %d", c.row, c.col, p_cursor, rope_len(buffer->text), p_mem);
    // output_cup(window->y + window->height - 1, window->x + 1);
    output_normal();
    // output_str(buf);

    // Cursor.
    // if (buffer->selections->size == 1) {
    //     Point c = rope_index_to_point(buffer->text, ((Selection*) buffer->selections->data[0])->cursor);
    //     output_cvvis();
    //     output_cup(c.row + window->y - view->scroll_line, c.col + window->x + ln_width - view->scroll_col);
    // } else {
        output_civis();
    // }

}
