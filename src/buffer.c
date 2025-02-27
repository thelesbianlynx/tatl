#include "buffer.h"

#include "array.h"
#include "charbuffer.h"
#include "output.h"

static int32_t cursor_fake_col (Buffer* buffer, CharBuffer* line, int32_t real_col);
static int32_t cursor_real_col (Buffer* buffer, CharBuffer* line, int32_t fake_col);
static void update_mem (Buffer* buffer);
static void update_scroll (Buffer* buffer);

Buffer* buffer_create (const char* title) {
    Buffer* buffer = malloc(sizeof(Buffer));

    buffer->lines = array_create();
    array_add(buffer->lines, charbuffer_create());

    buffer->cursor = buffer->selection = (Point) {0,0};

    buffer->col_mem = 0;
    buffer->scroll_damage = 0;
    buffer->scroll_line = 0;
    buffer->scroll_offset = 0;
    buffer->scroll_pages = 0;

    buffer->title = strdup(title);
    buffer->filename = NULL;

    buffer->alt_mode = false;
    buffer->page_mode = false;
    buffer->block_mode = false;
    buffer->raise_mode = false;

    buffer->tab_width = 4;
    buffer->hard_tabs = false;

    // Load contents.
    // FILE* f = fopen(file, "r");
    // for (;;) {
    //     CharBuffer* line = charbuffer_create();
    //     bool b = charbuffer_read_line(line, f);
    //     array_add (buffer->lines, line);
    //     if (!b) break;
    // }
    // fclose(f);

    return buffer;
}

void buffer_destroy (Buffer* buffer) {
    for (int i = 0; i < buffer->lines->size; ++i) {
        charbuffer_destroy(buffer->lines->data[i]);
    }
    array_destroy(buffer->lines);
    free(buffer->title);
    free(buffer->filename);
    free(buffer);
}

void buffer_load (Buffer* buffer, const char* filename) {
    for (int i = 0; i < buffer->lines->size; ++i) {
        charbuffer_destroy(buffer->lines->data[i]);
    }
    array_clear(buffer->lines);

    FILE* f = fopen(filename, "r");
    for (;;) {
        CharBuffer* line = charbuffer_create();
        bool b = charbuffer_read_line(line, f);
        array_add (buffer->lines, line);
        if (!b) break;
    }
    fclose(f);

    free(buffer->filename);
    buffer->filename = strdup(filename);
    free(buffer->title);
    buffer->title = strdup(filename);
}

bool buffer_save (Buffer* buffer) {

    return false;
}

void buffer_draw (Buffer* buffer, Box window, uint32_t mstate, uint32_t mx, uint32_t my) {
    // Line Number Width.
    int32_t ln_width = (int) log10(buffer->lines->size + 1) + 3;

    // Final Cursor Position.
    int32_t cx = -1,  cy = -1;

    // Handle Mouse Input.
    if (mx > window.x && my > window.y) {
        mx -= window.x;
        my -= window.y;
        if (mx <= window.width && my <= window.height) {
            if (mstate == 0) {
                // Press.
                int32_t row = my - 2;
                int32_t col = mx - ln_width - 2;
                if (row >= 0 && col >= 0) {
                    buffer_cursor_goto(buffer, row + buffer->scroll_line, col, false);
                }
            } else if (mstate == 32) {
                // Drag.
                int32_t row =  my - 2;
                int32_t col = mx - ln_width - 2;
                if (row >= 0 && col >= 0) {
                    buffer_cursor_goto(buffer, row + buffer->scroll_line, col, true);
                }
            } else if (mstate == 64) {
                // Scroll Up.
                buffer->scroll_line--;
                if (buffer->scroll_line < 0) {
                    buffer->scroll_line = 0;
                }
            } else if (mstate == 65) {
                // Scroll Down.
                buffer->scroll_line++;
                if (buffer->scroll_line > buffer->lines->size - 1) {
                    buffer->scroll_line = buffer->lines->size - 1;
                }
            }
        }
    }

    // Update Scroll Position.
    if (buffer->scroll_damage) {
        if (buffer->cursor.line < buffer->scroll_line) {
            buffer->scroll_line = buffer->cursor.line;
        } else if (buffer->cursor.line > buffer->scroll_line + window.height - 3) {
            buffer->scroll_line = buffer->cursor.line - window.height + 3;
        }
        buffer->scroll_damage = false;
    }

    // Selection Flag.
    bool in_selection = false;
    if (buffer->cursor.line < buffer->scroll_line) in_selection = !in_selection;
    if (buffer->selection.line < buffer->scroll_line) in_selection = !in_selection;

    // Title.
    output_cup(window.y, window.x);
    output_normal();
    output_underline();
    char title_buf[window.width + 1];
    snprintf(title_buf, window.width + 1, "%*c%-*s", ln_width - 2, ' ', window.width - ln_width + 2, buffer->title);
    output_str(title_buf);
    output_no_underline();

    // Buffer.
    for (int i = 0; i < window.height - 1; i++) {
        output_cup(window.y + i + 1, window.x);
        //output_setfg(2);
        output_bold();

        int lineno = i + buffer->scroll_line;

        //End of File.
        if (lineno >= buffer->lines->size) {
            // for (int j = 0; j < ln_width; j++) {
            //     output_char(' ');
            // }
            // output_normal();
            // for (int j = ln_width; j < window.width; j++) {
            //     output_char(' ');
            // }
            continue;
        }

        CharBuffer* line = buffer->lines->data[lineno];

        // Line Number.
        char ln_buf[ln_width + 1];
        snprintf(ln_buf, ln_width + 1, "%*d ", ln_width - 1 , lineno + 1);
        output_str(ln_buf);

        // Line Text.
        output_normal();
        if (in_selection) output_reverse();
        output_char(' ');

        int col = 0;
        for (int j = 0; j <= line->size; j++) {
            if (col + ln_width + 1 >= window.width) {
                if (buffer->cursor.line == i && buffer->cursor.col >= j) in_selection = !in_selection;
                if (buffer->selection.line == i && buffer->selection.col >= j) in_selection = !in_selection;
                break;
            }

            if (buffer->cursor.line == lineno && buffer->cursor.col == j) {
                in_selection = !in_selection;
                output_normal();
                if (in_selection) output_reverse();
                cx = col + ln_width + 1;
                cy = i + 1;
            }

            if (buffer->selection.line == lineno && buffer->selection.col == j) {
                in_selection = !in_selection;
                output_normal();
                if (in_selection) output_reverse();
            }

            if (j < line->size) {
                if (line->buffer[j] > 32) {
                    output_char(line->buffer[j]);
                    col++;
                } else if (line->buffer[j] == '\t') {
                    int32_t t = buffer->tab_width - MOD(col, buffer->tab_width);
                    for (int k = 0; k < t; k++) {
                        output_char(' ');
                        col++;
                    }
                } else {
                    output_char(' ');
                    col++;
                }
            }

        }

        // Rest Of Line.
        // if (in_selection) output_setbg(15);
        // for (int j = col + ln_width + 1; j < window.width; j++) {
        //     output_char(' ');
        // }
        output_normal();
    }

    // Set Final Cursor Position.
    if (cx >= 0 && cy >= 0) {
        output_cup(window.y + cy, window.x + cx);
        output_cvvis();
    } else {
        output_civis();
    }
}

static
bool delete_selection (Buffer* buffer) {
    Point begin = buffer->cursor, end = buffer->selection;

    if (begin.line == end.line) {
        if (begin.col == end.col) {
            return false;
        } else if (begin.col > end.col) {
            begin = buffer->selection;
            end = buffer->cursor;
        }
        CharBuffer* line = buffer->lines->data[begin.line];
        charbuffer_rm_substr(line, begin.col, end.col);
        buffer->cursor = buffer->selection = begin;
        update_mem(buffer);
        return true;
    } else if (begin.line > end.line) {
        begin = buffer->selection;
        end = buffer->cursor;
    }

    CharBuffer* line_begin = buffer->lines->data[begin.line];
    charbuffer_rm_suffix(line_begin, begin.col);

    for (int i = begin.line + 1; i < end.line; i++) {
        CharBuffer* line = array_remove(buffer->lines, begin.line + 1);
        charbuffer_destroy(line);
    }

    CharBuffer* line_end = array_remove(buffer->lines, begin.line + 1);
    charbuffer_rm_prefix(line_end, end.col);
    charbuffer_achars(line_begin, line_end);
    charbuffer_destroy(line_end);

    buffer->cursor = buffer->selection = begin;
    update_mem(buffer);
    update_scroll(buffer);
    return true;
}


void buffer_edit_char (Buffer* buffer, uint32_t ch, int32_t i) {
    delete_selection(buffer);
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        charbuffer_ichar(line, (char) ch, buffer->cursor.col);
        buffer->cursor.col++;
    }

    buffer->selection = buffer->cursor;
    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_edit_text (Buffer* buffer, CharBuffer* text, int32_t i) {
    delete_selection(buffer);
    while (i-- > 0) {
        for (int j = 0; j < text->size; j++) {
            uint32_t ch = text->buffer[j];
            if (ch == '\t' || ch >= 32) {
                buffer_edit_char(buffer, ch, 1);
            } else if (ch == '\n') {
                buffer_edit_line(buffer, 1);
            }
        }
    }
}

void buffer_edit_line (Buffer* buffer, int32_t i) {
    if (buffer->alt_mode) return;
    delete_selection(buffer);
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        CharBuffer* newline = charbuffer_create();
        charbuffer_get_suffix(line, newline, buffer->cursor.col);
        charbuffer_rm_suffix(line, buffer->cursor.col);
        array_insert(buffer->lines, buffer->cursor.line + 1, newline);
        buffer->cursor.line++;
        buffer->cursor.col = 0;
    }

    buffer->selection = buffer->cursor;
    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_edit_tab (Buffer* buffer, int32_t i) {
    if (buffer->alt_mode) return;
    if (buffer->cursor.line == buffer->selection.line && buffer->cursor.col == buffer->selection.col) {
        if (buffer->hard_tabs) {
            buffer_edit_char(buffer, '\t', i);
        } else {
            while (i-- > 0) {
                int c = cursor_fake_col(buffer, buffer->lines->data[buffer->cursor.line], buffer->cursor.col);
                buffer_edit_char(buffer, ' ', buffer->tab_width - MOD(c, buffer->tab_width));
            }
        }
    } else {
        buffer_edit_indent(buffer, i);
    }
}

void buffer_edit_indent (Buffer* buffer, int32_t i) {
    if (buffer->alt_mode) return;
    Point begin = buffer->cursor, end = buffer->selection;
    if (begin.line > end.line) {
        begin = buffer->selection;
        end = buffer->cursor;
    }

    if (i > 0) {
        while (i-- > 0) {
            if (buffer->hard_tabs) {
                for (int x = begin.line; x <= end.line; x++) {
                    CharBuffer* line = buffer->lines->data[x];
                    charbuffer_ichar(line, '\t', 0);
                }
                // Actually I think this is correct:
                buffer->cursor.col++;
                buffer->selection.col++;
            } else {
                for (int x = begin.line; x <= end.line; x++) {
                    CharBuffer* line = buffer->lines->data[x];
                    for (int y = 0; y < buffer->tab_width; y++)
                        charbuffer_ichar(line, ' ', 0);
                }
                buffer->cursor.col += buffer->tab_width;
                buffer->selection.col += buffer->tab_width;
            }
        }
    } else {
        while (i++ < 0) {
            for (int x = begin.line; x <= end.line; x++) {
                CharBuffer* line = buffer->lines->data[x];
                if (line->size == 0) continue;
                if (line->buffer[0] == '\t') {
                    charbuffer_rm_char(line, 0);
                    if (x == buffer->cursor.line) {
                        buffer->cursor.col--;
                        if (buffer->cursor.col < 0) {
                            buffer->cursor.col = 0;
                        }
                    }
                    if (x == buffer->selection.line) {
                        buffer->selection.col--;
                        if (buffer->selection.col < 0) {
                            buffer->selection.col = 0;
                        }
                    }
                }
                if (line->buffer[0] == ' ') {
                    int y;
                    for (y = 0; y < buffer->tab_width; y++) {
                        if (line->size == 0 || line->buffer[0] != ' ') break;
                        charbuffer_rm_char(line, 0);
                    }
                    if (x == buffer->cursor.line) {
                        buffer->cursor.col -= y;
                        if (buffer->cursor.col < 0) {
                            buffer->cursor.col = 0;
                        }
                    }
                    if (x == buffer->selection.line) {
                        buffer->selection.col -= y;
                        if (buffer->selection.col < 0) {
                            buffer->selection.col = 0;
                        }
                    }
                }
            }
        }
    }
    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_edit_delete (Buffer* buffer, int32_t i) {
    if (i > 0 && delete_selection(buffer)) i--;
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (buffer->cursor.col >= line->size) {
            if (buffer->alt_mode) return;
            if (buffer->cursor.line < buffer->lines->size - 1) {
                CharBuffer* nextline = array_remove(buffer->lines, buffer->cursor.line + 1);
                charbuffer_achars(line, nextline);
                charbuffer_destroy(nextline);
            }
        } else {
            charbuffer_rm_char(line, buffer->cursor.col);
        }
    }

    update_scroll(buffer);
}

void buffer_edit_backspace (Buffer* buffer, int32_t i) {
    if (i > 0 && delete_selection(buffer)) i--;
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (buffer->cursor.col <= 0) {
            if (buffer->alt_mode) return;
            if (buffer->cursor.line > 0) {
                CharBuffer* prevline = buffer->lines->data[buffer->cursor.line - 1];
                buffer->cursor.col = prevline->size;
                array_remove(buffer->lines, buffer->cursor.line);
                charbuffer_achars(prevline, line);
                charbuffer_destroy(line);
                buffer->cursor.line--;
            }
        } else {
            charbuffer_rm_char(line, buffer->cursor.col - 1);
            buffer->cursor.col--;
        }
    }

    buffer->selection = buffer->cursor;
    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_edit_move_line (Buffer* buffer, int32_t i) {
    if (buffer->alt_mode) return;
    if (i < 0) {
        i = -i;
        while (i-- > 0) {
            int top = buffer->cursor.line;
            int bot = buffer->selection.line;
            if (top > bot) {
                int swap = top;
                top = bot;
                bot = swap;
            }
            if (top <= 0) break;
            CharBuffer* line = array_remove(buffer->lines, top - 1);
            array_insert(buffer->lines, bot, line);
            buffer->cursor.line--;
            buffer->selection.line--;
        }
    } else if (i > 0) {
        while (i-- > 0) {
            int top = buffer->cursor.line;
            int bot = buffer->selection.line;
            if (top > bot) {
                int swap = top;
                top = bot;
                bot = swap;
            }
            if (bot >= buffer->lines->size - 1) break;
            CharBuffer* line = array_remove(buffer->lines, bot + 1);
            array_insert(buffer->lines, top, line);
            buffer->cursor.line++;
            buffer->selection.line++;
        }
    }
}



void buffer_cursor_goto (Buffer* buffer, int32_t row, int32_t col, bool sel) {
    if (row >= buffer->lines->size) row = buffer->lines->size - 1;
    if (buffer->alt_mode) row = 0;

    CharBuffer* line = buffer->lines->data[row];
    if (col > line->size) col = line->size;

    buffer->cursor.line = row;
    buffer->cursor.col = col;

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line (Buffer* buffer, int32_t i, bool sel) {
    if (buffer->alt_mode) return;
    buffer->cursor.line += i;
    if (buffer->cursor.line <= 0) buffer->cursor.line = 0;
    if (buffer->cursor.line >= buffer->lines->size) buffer->cursor.line = buffer->lines->size - 1;

    // Line Wrapping logic goes here...
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    buffer->cursor.col = cursor_real_col(buffer, line, buffer->col_mem);

    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_char (Buffer* buffer, int32_t i, bool sel) {
    int d = i > 0 ? 1 : -1;
    if (i < 0) i *= -1;

    while (i-- > 0) {
        int c = (int) buffer->cursor.col + d;

        if (c < 0) {
            if (buffer->alt_mode) break;
            if (buffer->cursor.line <= 0) break;
            buffer->cursor.line--;
            CharBuffer* line = buffer->lines->data[buffer->cursor.line];
            buffer->cursor.col = line->size;
            continue;
        }

        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (c > line->size) {
            if (buffer->alt_mode) break;
            if (buffer->cursor.line >= buffer->lines->size - 1) break;
            buffer->cursor.line++;
            buffer->cursor.col = 0;
            continue;
        }

        buffer->cursor.col = c;
    }

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

static
uint32_t chartype (char ch) {
    // Whitespace.
    if (ch <= ' ')
        return 0;

    // Text (Letters and Numbers).
    if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'Z' ) || ('a' <= ch && ch <= 'z') || ch > 127)
        return 1;

    // Symbol (The rest).
    return 2;
}

void buffer_cursor_word (Buffer* buffer, int32_t lead, int32_t i, bool sel) {
    int d = i > 0 ? 1 : -1;
    if (i < 0) i *= -1;

    while (i-- > 0) {
        // Leading Move.
        buffer_cursor_char(buffer, d * lead, true);

        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        char ch = line->buffer[buffer->cursor.col - (d > 0 ? 0 : 1)];
        uint32_t type = chartype(ch);

        // Forward.
        if (d > 0) {
            for (;;) {
                buffer->cursor.col++;
                if (buffer->cursor.col >= line->size) {
                    buffer->cursor.col = line->size;
                    break;
                }

                char c = line->buffer[buffer->cursor.col];
                uint32_t t = chartype(c);
                if (t != type) {
                    break;
                }
            }
        }

        // Backward.
        if (d < 0) {
            for (;;) {
                buffer->cursor.col--;
                if (buffer->cursor.col <= 0) {
                    buffer->cursor.col = 0;
                    break;
                }

                char c = line->buffer[buffer->cursor.col - 1];
                uint32_t t = chartype(c);
                if (t != type) {
                    break;
                }
            }
        }
    }

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_paragraph (Buffer* buffer, int32_t i, bool sel) {
    if (buffer->alt_mode) return;

}

void buffer_cursor_home (Buffer* buffer, bool sel) {
    buffer->cursor.line = 0;
    buffer->cursor.col = 0;

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_end (Buffer* buffer, bool sel) {
    buffer->cursor.line = buffer->lines->size - 1;
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    buffer->cursor.col = line->size;

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line_begin (Buffer* buffer, bool sel) {
    buffer->cursor.col = 0;

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line_end (Buffer* buffer, bool sel) {
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    buffer->cursor.col = line->size;

    update_mem(buffer);
    update_scroll(buffer);

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}



void buffer_select_all (Buffer* buffer) {
    buffer_cursor_goto(buffer, 0, 0, false);
    buffer_cursor_goto(buffer, INT_MAX, INT_MAX, true);

    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_select_word (Buffer* buffer) {
    if (buffer_selection_exist(buffer)) {
        buffer_selection_clear(buffer);
        return;
    }

    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    uint32_t type = chartype(line->buffer[buffer->cursor.col]);

    int32_t min = buffer->cursor.col;
    int32_t max = min;

    for (;;) {
        if (min <= 0) break;
        if (chartype(line->buffer[min - 1]) != type) break;
        min--;
    }

    for (;;) {
        if (max >= line->size) break;
        if (chartype(line->buffer[max]) != type) break;
        max++;
    }

    buffer->cursor.col = max;
    buffer->selection.col = min;

    update_mem(buffer);
    update_scroll(buffer);
}

void buffer_select_line (Buffer* buffer) {
    if (buffer->cursor.line == buffer->selection.line) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (buffer->cursor.col >= buffer->selection.col) {
            if (buffer->cursor.col < line->size || buffer->selection.col > 0) {
                buffer->cursor.col = line->size;
                buffer->selection.col = 0;
            } else {
                // Clamp to non-whitespace.
                // ...
            }
        } else {
            if (buffer->cursor.col > 0 || buffer->selection.col < line->size) {
                buffer->cursor.col = 0;
                buffer->selection.col = line->size;
            } else {
                // Clamp to non-whitespace.
                // ...
            }
        }
    } else if (buffer->cursor.line > buffer->selection.line) {
        CharBuffer* cline = buffer->lines->data[buffer->cursor.line];
        //CharBuffer* sline = buffer->lines->data[buffer->selection.line];
        if (buffer->cursor.col < cline->size || buffer->selection.col > 0) {
            buffer->cursor.col = cline->size;
            buffer->selection.col = 0;
        } else {
            // Clamp to non-whitespace.
            // ...
        }
    } else {
        //CharBuffer* cline = buffer->lines->data[buffer->cursor.line];
        CharBuffer* sline = buffer->lines->data[buffer->selection.line];
        if (buffer->cursor.col > 0 || buffer->selection.col < sline->size) {
            buffer->cursor.col = 0;
            buffer->selection.col = sline->size;
        } else {
            // Clamp to non-whitespace.
            // ...
        }
    }
}


bool buffer_selection_exist (Buffer* buffer) {
    return !(buffer->cursor.line == buffer->selection.line && buffer->cursor.col == buffer->selection.col);
}

void buffer_selection_swap (Buffer* buffer) {
    Point tmp = buffer->cursor;
    buffer->cursor = buffer->selection;
    buffer->selection = tmp;
}

void buffer_selection_clear (Buffer* buffer) {
    buffer->selection = buffer->cursor;
}

void buffer_selection_copy (Buffer* buffer, CharBuffer* dst) {
    Point begin = buffer->cursor, end = buffer->selection;

    if (begin.line == end.line) {
        if (begin.col > end.col) {
            begin = buffer->selection;
            end = buffer->cursor;
        }
        CharBuffer* line = buffer->lines->data[begin.line];
        charbuffer_get_substr(line, dst, begin.col, end.col);
    } else {
        if (begin.line > end.line) {
            begin = buffer->selection;
            end = buffer->cursor;
        }

        CharBuffer* line_begin = buffer->lines->data[begin.line];
        charbuffer_get_suffix(line_begin, dst, begin.col);

        for (int i = begin.line + 1; i < end.line; i++) {
            CharBuffer* line = buffer->lines->data[i];
            charbuffer_achar(dst, '\n');
            charbuffer_achars(dst, line);
        }

        CharBuffer* line_end = buffer->lines->data[end.line];
        charbuffer_achar(dst, '\n');
        charbuffer_get_prefix(line_end, dst, end.col);
    }
}

void buffer_selection_cut (Buffer* buffer, CharBuffer* dst) {
    buffer_selection_copy(buffer, dst);
    delete_selection(buffer);
}

void buffer_selection_duplicate (Buffer* buffer, int32_t i) {

}

void buffer_selection_delete_whitespace (Buffer* buffer) {

}


void buffer_undo (Buffer* buffer, int32_t i) {}

void buffer_redo (Buffer* buffer, int32_t i) {}


static
int32_t cursor_real_col (Buffer* buffer, CharBuffer* line, int32_t fake_col) {
    for (int i = 0; i < line->size; i++) {
        if (line->buffer[i] == '\t') {
            fake_col -= buffer->tab_width - MOD(i, buffer->tab_width);
        } else {
            fake_col--;
        }
        if (fake_col < 0) return i;
    }
    return line->size;
}

static
int32_t cursor_fake_col (Buffer* buffer, CharBuffer* line, int32_t real_col) {
    int32_t r = 0;
    for (int i = 0; i < real_col; i++) {
        if (i >= line->size) break;
        if (line->buffer[i] == '\t') {
            r += buffer->tab_width - MOD(i, buffer->tab_width);
        } else {
            r++;
        }
    }
    return r;
}

static
void update_mem (Buffer* buffer) {
    buffer->col_mem = cursor_fake_col(buffer, buffer->lines->data[buffer->cursor.line], buffer->cursor.col);
}

static
void update_scroll (Buffer* buffer) {
    buffer->scroll_damage = true;
}
