#include "buffer.h"

#include "array.h"
#include "charbuffer.h"

Buffer* buffer_create (char* file) {
    Buffer* buffer = malloc(sizeof(Buffer));

    buffer->lines = array_create();

    buffer->cursor = buffer->selection = (Point) {0,0};

    buffer->filename = file;

    buffer->tab_width = 4;
    buffer->hard_tabs = false;

    // Load contents.
    FILE* f = fopen(file, "r");
    for (;;) {
        CharBuffer* line = charbuffer_create();
        bool b = charbuffer_read_line(line, f);
        array_add (buffer->lines, line);
        if (!b) break;
    }
    fclose(f);

    return buffer;
}

void buffer_destroy (Buffer* buffer) {
    for (int i = 0; i < buffer->lines->size; ++i) {
        charbuffer_destroy(buffer->lines->data[i]);
    }
    array_destroy(buffer->lines);
    free(buffer);
}

void buffer_save (Buffer* buffer) {

}

void buffer_draw (Buffer* buffer, Box window) {
    // Line Number Width.
    int32_t ln_width = (int) log10(buffer->lines->size) + 3;

    // Final Cursor Position.
    int32_t cx = -1,  cy = -1;

    // Selection Flag.
    bool in_selection = false;

    // Title.
    output_cup(window.y, window.x);
    output_setfg(0);
    output_setbg(7);
    char title_buf[window.width + 1];
    snprintf(title_buf, window.width + 1, "%*c%-*s", ln_width - 2, ' ', window.width - ln_width - 2, buffer->filename);
    output_str(title_buf);

    // Buffer.
    for (int i = 0; i < window.height - 1; i++) {
        output_cup(window.y + i + 1, window.x);
        output_setfg(7);
        output_setbg(8);

        //End of File.
        if (i >= buffer->lines->size) {
            for (int j = 0; j < ln_width; j++) {
                output_char(' ');
            }
            output_setfg(0);
            output_setbg(15);
            for (int j = ln_width; j < window.width; j++) {
                output_char(' ');
            }
            continue;
        }

        CharBuffer* line = buffer->lines->data[i];

        // Line Number.
        char ln_buf[ln_width + 1];
        snprintf(ln_buf, ln_width + 1, "%*d ", ln_width - 1 , i);
        output_str(ln_buf);

        // Line Text.
        output_setfg(0);
        output_setbg(in_selection ? 14 : 15);
        output_char(' ');
        for (int j = 0; j <= line->size; j++) {
            if (j + ln_width + 1 >= window.width) {
                if (buffer->cursor.line == i && buffer->cursor.col >= j) in_selection = !in_selection;
                if (buffer->selection.line == i && buffer->selection.col >= j) in_selection = !in_selection;
                break;
            }

            if (buffer->cursor.line == i && buffer->cursor.col == j) {
                in_selection = !in_selection;
                output_setbg(in_selection ? 14 : 15);
                cx = ln_width + 1 + j;
                cy = i + 1;
            }

            if (buffer->selection.line == i && buffer->selection.col == j) {
                in_selection = !in_selection;
                output_setbg(in_selection ? 14 : 15);
            }

            if (line->buffer[j] > 32) {
                output_char(line->buffer[j]);
            } else if (j < line->size) {
                output_char(' ');
            }
        }

        // Rest Of Line.
        if (in_selection) output_setbg(15);
        for (int j = ln_width + 1 + line->size; j < window.width; j++) {
            output_char(' ');
        }
    }

    // Set Final Cursor Position.
    if (cx >= 0 && cy >= 0) {
        output_cup(window.y + cy, window.x + cx);
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
}

void buffer_edit_text (Buffer* buffer, CharBuffer* text, int32_t i) {

    // ...
}

void buffer_edit_line (Buffer* buffer, int32_t i) {
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
}

void buffer_edit_tab (Buffer* buffer, int32_t i) {

    // ...
}

void buffer_edit_indent (Buffer* buffer, int32_t i) {

    // ...
}

void buffer_edit_delete (Buffer* buffer, int32_t i) {
    if (i > 0 && delete_selection(buffer)) i--;
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (buffer->cursor.col >= line->size) {
            if (buffer->cursor.line < buffer->lines->size - 1) {
                CharBuffer* nextline = array_remove(buffer->lines, buffer->cursor.line + 1);
                charbuffer_achars(line, nextline);
                charbuffer_destroy(nextline);
            }
        } else {
            charbuffer_rm_char(line, buffer->cursor.col);
        }
    }
}

void buffer_edit_backspace (Buffer* buffer, int32_t i) {
    if (i > 0 && delete_selection(buffer)) i--;
    while (i-- > 0) {
        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (buffer->cursor.col <= 0) {
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
}



void buffer_cursor_goto (Buffer* buffer, int32_t row, int32_t col, bool sel) {
    if (row >= buffer->lines->size) row = buffer->lines->size - 1;

    CharBuffer* line = buffer->lines->data[row];
    if (col > line->size) col = line->size;

    buffer->cursor.line = row;
    buffer->cursor.col = col;

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line (Buffer* buffer, int32_t i, bool sel) {
    buffer->cursor.line += i;
    if (buffer->cursor.line <= 0) buffer->cursor.line = 0;
    if (buffer->cursor.line >= buffer->lines->size) buffer->cursor.line = buffer->lines->size - 1;

    // Line Wrapping logic goes here...
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    if (buffer->cursor.col > line->size) buffer->cursor.col = line->size;

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
            if (buffer->cursor.line <= 0) {
                break;
            }
            buffer->cursor.line--;
            CharBuffer* line = buffer->lines->data[buffer->cursor.line];
            buffer->cursor.col = line->size;
            continue;
        }

        CharBuffer* line = buffer->lines->data[buffer->cursor.line];
        if (c > line->size) {
            if (buffer->cursor.line >= buffer->lines->size - 1) {
                break;
            }
            buffer->cursor.line++;
            buffer->cursor.col = 0;
            continue;
        }

        buffer->cursor.col = c;
    }

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_word (Buffer* buffer, int32_t i, bool sel) {

    // ...
}

void buffer_cursor_home (Buffer* buffer, bool sel) {
    buffer->cursor.line = 0;
    buffer->cursor.col = 0;

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_end (Buffer* buffer, bool sel) {
    buffer->cursor.line = buffer->lines->size - 1;
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    buffer->cursor.col = line->size;

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line_begin (Buffer* buffer, bool sel) {
    buffer->cursor.col = 0;

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}

void buffer_cursor_line_end (Buffer* buffer, bool sel) {
    CharBuffer* line = buffer->lines->data[buffer->cursor.line];
    buffer->cursor.col = line->size;

    if (!sel) {
        buffer->selection = buffer->cursor;
    }
}
