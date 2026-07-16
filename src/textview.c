#include "textview.h"

#include "array.h"
#include "input.h"
#include "output.h"
#include "rope.h"
#include "textbuffer.h"
#include "colorizer.h"
#include "mode.h"


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
    int32_t* styles;
    
    uint32_t col_start;
    uint32_t col_end;
    uint32_t col;

    uint32_t tab_width;
    Colorizer* colorizer;
    Array* selections;
};

static
bool char_style (uint32_t i, uint32_t ch, void* d) {
    struct draw_char_data* data = d;
    //if (data->col >= data->col_end) return true;

    // Cursor Style.
    int32_t style = 0;
    for (int x = 0; x < data->selections->size; x++) {
        Selection* sel = data->selections->data[x];
        if (head(sel) <= i && i < tail(sel)) style |= STYLE_SELECTION;
        if (i == sel->cursor && !cursor_blink) style |= STYLE_CURSOR;
    }

    // -- Emit Character and Style -- //

    // Special case: Hard Tab.
    if (ch == '\t') {
        int t = data->tab_width - (data->col % data->tab_width);
        for (int x = 0; x < t; x++) {
            if (data->col < data->col_end && data->col >= data->col_start) {
                uint32_t c = data->col - data->col_start;
                data->chars[c] = ' ';
                data->styles[c] |= style;
            }
            data->col++;
        }
    }
    // Special case: Control Character (print as whitespace for now).
    else if (ch < 32) {
        if (data->col < data->col_end && data->col >= data->col_start) {
            uint32_t c = data->col - data->col_start;
            data->chars[c] = ' ';
            data->styles[c] |= style;
        }
        data->col++;
    }
    // General case: Regular Character.
    else {
        if (data->col < data->col_end && data->col >= data->col_start) {
            uint32_t c = data->col - data->col_start;
            data->chars[c] = ch;
            data->styles[c] |= style;
        }
        data->col++;
    }

    // Colorizer Logic.
    colorize_next_char(data->colorizer, ch, data->col, data->col_start, data->col_end, data->styles);

    return true;
}

static
bool char_style_fast (uint32_t i, uint32_t ch, void* d) {
    Colorizer* colorizer = d;
    colorize_next_char_fast(colorizer, ch);
    return true;
}

static
int scroll_len (double dtime) {
    int r = (int) (0.05/dtime);
    if (r <= 0) r = 1;
    return r;
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
                    view->scroll_line -= MIN(5, scroll_len(mstate->dtime));
                } else if (mstate->button == 65) {
                    // Scroll Down.
                    view->scroll_line += MIN(5, scroll_len(mstate->dtime));
                }
            }
        }
    }

    // Update Scroll.
    if (buffer->cursor_dmg) {
        // Which cursor to follow.
        Selection* sel = buffer->selections->data[0];
        if (sel->primary) sel = array_peek(buffer->selections);

        // Scroll Line.
        int32_t row = rope_index_to_point(buffer->text, sel->cursor).row;
        if (text_height == 1) {
            view->scroll_line = row;
        } else if (row < view->scroll_line) {
            view->scroll_line = row;
        } else if (row > view->scroll_line + text_height - 2) {
            view->scroll_line = MAX(0, row - text_height + 2);
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

    // Colorizer data.
    Colorizer colorizer = { .mode = buffer->mode };

    // Pre-fill buffer line-state array up to first line.
    while (buffer->line_state->size < view->scroll_line) {
        // Line number to fill in.
        int32_t line = buffer->line_state->size;

        // Line Start State.
        int32_t start_state = 0;
        if (line > 0) {
            start_state = (int32_t)(intptr_t) buffer->line_state->data[line-1];
        }

        // Line Start and End.
        int32_t start = rope_point_to_index(buffer->text, (Point) {line, 0});
        int32_t end = rope_point_to_index(buffer->text, (Point) {line, INT_MAX});

        // Fill in.
        colorize_begin_line(&colorizer, start_state);
        rope_foreach_substr(buffer->text, start, end, char_style_fast, &colorizer);
        char_style_fast(end, '\n', &colorizer);
        array_add(buffer->line_state, (void*)(intptr_t) colorizer.comment_depth);
    }

    for (int i = 0; i < text_height; i++) {

        // End of Buffer.
        if (view->scroll_line + i >= lines) {
            break;
        }

        // Start and end of line.
        int32_t start = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i, 0});
        int32_t end = rope_point_to_index(buffer->text, (Point) {view->scroll_line + i, INT_MAX});

        // Line Start State.
        int32_t start_state = 0;
        assert(buffer->line_state->size >= view->scroll_line + i && "Invalid line state array");
        if (view->scroll_line + i > 0) {
            start_state = (int32_t)(intptr_t) buffer->line_state->data[view->scroll_line + i - 1];
        }

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
            .chars = chars,
            .styles = styles,
            .col_start = view->scroll_col,
            .col_end = view->scroll_col + text_width,
            .tab_width = buffer->tab_width,
            .selections = buffer->selections,
            .colorizer = &colorizer,
        };

        // Get Line Content and Style.
        colorize_begin_line(&colorizer, start_state);
        rope_foreach_substr(buffer->text, start, end, char_style, &data);
        char_style(end, '\n', &data);

        // Buffer line state if not filled in.
        if (buffer->line_state->size <= view->scroll_line + i)
            array_add(buffer->line_state, (void*) (intptr_t) colorizer.comment_depth);

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
                current_style = style;

                if (style & STYLE_SELECTION) {
                    output_setbg(12);
                }
                if (style & STYLE_CURSOR) {
                    output_underline();
                }
                if (style & STYLE_SYMBOL) {
                    output_bold();
                    output_setfg(14);
                }
                if (style & STYLE_KEYWORD) {
                    output_bold();
                    output_setfg(11);
                }
                if (style & STYLE_NAME) {
                    output_setfg(10);
                }
                if (style & STYLE_COMMENT) {
                    output_italic();
                    output_setfg(13);
                }
                if (style & STYLE_STRING) {
                    output_setfg(9);
                }
                if (style & STYLE_CHAR) {
                    output_setfg(3);
                }
            }
            // Put character.
            assert(ch >= 32 && "Invalid Ouput Character");
            output_uchar(ch);
        }
    }

    output_normal();
    output_civis();
}
