#include "textbuffer.h"

#include "array.h"
#include "intbuffer.h"
#include "rope.h"


//
// Predeclarations.
//

static void action_begin (TextBuffer*, uint32_t);
static void action_end (TextBuffer*);

//
// Enums.
//

// Character Type.
enum {
    CHARTYPE_WS,
    CHARTYPE_TEXT,
    CHARTYPE_SYMBOL,
};

// Action Type.
enum {
    ACTION_CHAR0 = CHARTYPE_WS,
    ACTION_CHAR1 = CHARTYPE_TEXT,
    ACTION_CHAR2 = CHARTYPE_SYMBOL,

    ACTION_EDIT,

    ACTION_INDENT,
    ACTION_UNINDENT,
    ACTION_DELETE,
    ACTION_DELETE_LINES,
    ACTION_BACKSPACE,
    ACTION_BACKSPACE_LINES,
    ACTION_MOVE,
    ACTION_MOVE_LINES,

};


//
// Selection Object.
//

static
Selection* selection_create () {
    Selection* sel = malloc(sizeof(Selection));
    sel->cursor = 0;
    sel->anchor = 0;
    sel->col_mem = 0;
    sel->primary = false;

    return sel;
}

static
Selection* selection_copy (Selection* s) {
    Selection* sel = malloc(sizeof(Selection));
    sel->cursor = s->cursor;
    sel->anchor = s->anchor;
    sel->col_mem = s->col_mem;
    sel->primary = false;

    return sel;
}

static
void selection_destroy (Selection* sel) {
    free(sel);
}

//
// Selection Macros,
//

static inline
uint32_t head (Selection* sel) {
    return MIN(sel->cursor, sel->anchor);
}

static inline
uint32_t tail (Selection* sel) {
    return MAX(sel->cursor, sel->anchor);
}

static inline
void selection_clamp (Rope* text, Selection* sel) {
    int32_t len = rope_len(text);
    sel->cursor = MIN(0, MAX(sel->cursor, len));
    sel->anchor = MIN(0, MAX(sel->anchor, len));
}

static inline
uint32_t selection_len (Selection* sel) {
    return ABS(sel->cursor - sel->anchor);
}

static inline
uint32_t selection_array_max_len (Array* A) {
    uint32_t len = 0;
    for (int i = 0; i < A->size; i++)
        len = MAX(len, selection_len(A->data[i]));
    return len;
}

static inline
void selection_array_copy (Array* src, Array* dst) {
    for (int i = 0; i < src->size; i++) {
        Selection* sel = src->data[i];
        Selection* cpy = selection_copy(sel);
        cpy->primary = sel->primary;
        array_add(dst, cpy);
    }
}

//  -> Returns Primary selection instead of destoying it.
//      -> Only Returns the first one marked primary.
static inline
Selection* selection_array_clear (Array* A) {
    Selection* primary_sel = NULL;
    for (int i = 0; i < A->size; i++) {
        Selection* sel = A->data[i];
        if (sel->primary && primary_sel == NULL) {
            primary_sel = sel;
        } else {
            selection_destroy(sel);
        }
    }
    array_clear(A);
    return primary_sel;
}

//
// History Object.
//

typedef struct hist Hist;
struct hist {
    Rope* text;

    Array* selections;
    Array* pre_selections;
};

static
Hist* hist_create (TextBuffer* buffer) {
    Hist* hist = malloc(sizeof(Hist));
    hist->text = rope_copy(buffer->text);
    hist->selections = array_create();
    hist->pre_selections = array_create();

    selection_array_copy(buffer->selections, hist->selections);
    selection_array_copy(buffer->pre_selections, hist->pre_selections);

    return hist;
}

static
void hist_destroy (Hist* hist) {
    selection_destroy(selection_array_clear(hist->selections));
    selection_destroy(selection_array_clear(hist->pre_selections));
    array_destroy(hist->selections);
    array_destroy(hist->pre_selections);
    rope_destroy(hist->text);
    free(hist);
}

static inline
void hist_array_clear (Array* A) {
    for (int i = 0; i < A->size; i++) {
        hist_destroy(A->data[i]);
    }
    array_clear(A);
}

//
// TextBuffer Object.
//

TextBuffer* textbuffer_create (Rope* text) {
    TextBuffer* buffer = malloc(sizeof(TextBuffer));
    buffer->text = text;
    buffer->undo = array_create();
    buffer->redo = array_create();
    buffer->action_state = false;
    buffer->action_type = 0;
    buffer->selections = array_create();
    buffer->pre_selections = array_create();
    buffer->tab_width = 4;
    buffer->hard_tabs = false;
    buffer->altmode = false;
    buffer->cursor_dmg = false;
    buffer->text_dmg = false;

    Selection* primary_sel = selection_create();
    primary_sel->primary = true;
    array_add(buffer->selections, primary_sel);

    action_begin(buffer, ACTION_EDIT);
    action_end(buffer);

    return buffer;
}

void textbuffer_destroy (TextBuffer* buffer) {
    selection_destroy(selection_array_clear(buffer->selections));
    selection_destroy(selection_array_clear(buffer->pre_selections));
    array_destroy(buffer->selections);
    array_destroy(buffer->pre_selections);
    hist_array_clear(buffer->undo);
    hist_array_clear(buffer->redo);
    array_destroy(buffer->undo);
    array_destroy(buffer->redo);
    rope_destroy(buffer->text);
    free(buffer);
}

//
// Undo/Redo and Action tracking.
//

static
void action_begin (TextBuffer* buffer, uint32_t action) {
    if (buffer->action_state && buffer->action_type != action) {
        action_end(buffer);
    }
    if (!buffer->action_state) {
        assert(buffer->pre_selections->size == 0 && "Invalid Action-Begin");
        selection_array_copy(buffer->selections, buffer->pre_selections);

        buffer->action_state = true;
        buffer->action_type = action;
    }

    buffer->text_dmg = true;
    buffer->cursor_dmg = true;
}

static
void action_end (TextBuffer* buffer) {
    if (buffer->action_state) {
        // Commit new State.
        Hist* state = hist_create(buffer);
        array_add(buffer->undo, state);
        hist_array_clear(buffer->redo);

        // History Limit.
        while (buffer->undo->size > HIST_LIMIT) {
            // Remove oldest.
            hist_destroy(array_remove(buffer->undo, 0));
        }

        selection_destroy(selection_array_clear(buffer->pre_selections));
        buffer->action_state = false;
    }

    buffer->cursor_dmg = true;
}


void textbuffer_undo (TextBuffer* buffer) {
    action_end(buffer);

    // Current state is always top of undo stack.
    // Pop off current state, push to redo, restore new current (new top of stack).
    // This means there must be at least two states in the undo stack.
    if (buffer->undo->size >= 2) {
        // Pop State.
        Hist* current = array_pop(buffer->undo);
        array_add(buffer->redo, current);

        // Restore state.
        Hist* state = array_peek(buffer->undo);

        // -> Text.
        rope_destroy(buffer->text);
        buffer->text = rope_copy(state->text);

        // -> Selections.
        selection_destroy(selection_array_clear(buffer->selections));
        selection_array_copy(current->pre_selections, buffer->selections);
    }
}

void textbuffer_redo (TextBuffer* buffer) {
    action_end(buffer);

    // Pop state off redo stack, resore it, and push to undo.
    // Undo state must have size at least 1.
    if (buffer->redo->size >= 1) {
        // Restore state.
        Hist* state = array_pop(buffer->redo);
        array_add(buffer->undo, state);

        // -> Text.
        rope_destroy(buffer->text);
        buffer->text = rope_copy(state->text);

        // -> Selections.
        selection_destroy(selection_array_clear(buffer->selections));
        selection_array_copy(state->selections, buffer->selections);
    }
}

//
// Edit Actions.
//

// -- Cursor Manipulation -- //

static
void update_selections (TextBuffer* buffer, uint32_t index, int32_t window) {
    for (int i = 0; i < buffer->selections->size; i++) {
        Selection* sel = buffer->selections->data[i];
        if (sel->cursor >= index) {
            sel->cursor = MAX(index, sel->cursor + window);
            sel->col_mem = rope_index_to_point(buffer->text, sel->cursor).col;
        }
        if (sel->anchor >= index) {
            sel->anchor = MAX(index, sel->anchor + window);
        }
    }
}

static
void textbuffer_edit (TextBuffer* buffer, uint32_t i, uint32_t j, Rope* text) {
    if (i > j) i = j;
    int32_t window = i - j;
    Rope* a = rope_prefix(buffer->text, i);
    Rope* b = rope_suffix(buffer->text, j);
    Rope* c;

    if (text == NULL) {
        c = rope_append(a, b);
    } else {
        Rope* d;
        d = rope_append(a, text);
        c = rope_append(d, b);
        rope_destroy(d);
        window += rope_len(text);
    }

    rope_destroy(a);
    rope_destroy(b);

    rope_destroy(buffer->text);
    buffer->text = c;

    update_selections(buffer, i, window);
}


// - Text Actions - //

static
uint32_t chartype (uint32_t ch) {
    // Whitespace.
    if (ch <= ' ')
        return CHARTYPE_WS;

    // Text (Letters and Numbers).
    if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'Z' ) || ('a' <= ch && ch <= 'z') || ch == '_' || ch > 127)
        return CHARTYPE_TEXT;

    // Symbol (The rest).
    return CHARTYPE_SYMBOL;
}

void textbuffer_edit_char (TextBuffer* buffer, uint32_t ch, int32_t i) {
    action_begin(buffer, chartype(ch));

    IntBuffer* itext = intbuffer_create();
    intbuffer_put_char(itext, 0, ch);

    Rope* text = rope_create(itext);
    intbuffer_destroy(itext);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        textbuffer_edit(buffer, head(sel), tail(sel), text);
        sel->anchor = sel->cursor;
    }

    rope_destroy(text);

    // Continuous action.
    // No Action-End.
}

void textbuffer_edit_text (TextBuffer* buffer, Rope* text, int32_t i) {
    action_begin(buffer, ACTION_EDIT);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        textbuffer_edit(buffer, head(sel), tail(sel), text);
        //sel->anchor = sel->cursor;
    }

    // Atomic Action.
    action_end(buffer);
}

void textbuffer_edit_newline (TextBuffer* buffer, int32_t i) {
    // Simply insert newline.
    textbuffer_edit_char(buffer, '\n', i);
}

void textbuffer_edit_tab (TextBuffer* buffer, int32_t i) {

}

void textbuffer_edit_indent (TextBuffer* buffer, int32_t i) {

}

// -- Delete Actions -- //

void textbuffer_edit_delete (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_DELETE);

    int32_t len = selection_array_max_len(buffer->selections);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        if (len > 0) {
            textbuffer_edit(buffer, head(sel), tail(sel), NULL);
        } else {
            textbuffer_edit(buffer, head(sel), tail(sel) + 1, NULL);
        }
    }

    // Continuous action.
    // No Action-End.
}

void textbuffer_edit_delete_lines (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_DELETE_LINES);
    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        Point phead = rope_index_to_point(buffer->text, head(sel));
        Point ptail = rope_index_to_point(buffer->text, tail(sel));
        int32_t head = rope_point_to_index(buffer->text, (Point) {phead.row, 0});
        int32_t tail = rope_point_to_index(buffer->text, (Point) {ptail.row + 1, 0});

        textbuffer_edit(buffer, head, tail, NULL);
    }

    // Continuous Action.
    // No Action End.
}

void textbuffer_edit_backspace (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_BACKSPACE);

    int32_t len = selection_array_max_len(buffer->selections);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        if (len > 0) {
            textbuffer_edit(buffer, head(sel), tail(sel), NULL);
        } else {
            textbuffer_edit(buffer, head(sel) - 1, tail(sel), NULL);
        }
    }

    // Continuous action.
    // No Action-End.
}

void textbuffer_edit_backspace_lines (TextBuffer* buffer, int32_t i) {

}

// - Other Actions - //

void textbuffer_edit_duplicate (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_EDIT);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        Rope* text = rope_substr(buffer->text, head(sel), tail(sel));
        textbuffer_edit(buffer, head(sel), head(sel), text);
    }

    // Atomic Action.
    action_end(buffer);
}

void textbuffer_edit_duplicate_lines (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_EDIT);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        Point phead = rope_index_to_point(buffer->text, head(sel));
        Point ptail = rope_index_to_point(buffer->text, tail(sel));
        int32_t head = rope_point_to_index(buffer->text, (Point) {phead.row, 0});
        int32_t tail = rope_point_to_index(buffer->text, (Point) {ptail.row + 1, 0});

        Rope* text = rope_substr(buffer->text, head, tail);
        textbuffer_edit(buffer, head, head, text);
    }

    // Atomic Action.
    action_end(buffer);
}

void textbuffer_edit_move_lines (TextBuffer* buffer, int32_t i) {
    action_begin(buffer, ACTION_MOVE_LINES);

    // Move lines down.
    while (i > 0) {
        int32_t top = rope_lines(buffer->text);
        int32_t bot = 0;
        for (int x = 0; x < buffer->selections->size; x++) {
            Selection* sel = buffer->selections->data[x];
            Point phead = rope_index_to_point(buffer->text, head(sel));
            Point ptail = rope_index_to_point(buffer->text, tail(sel));
            top = MIN(phead.row, top);
            bot = MAX(ptail.row, bot);
        }

        if (bot >= rope_lines(buffer->text)) break;

        int32_t head = rope_point_to_index(buffer->text, (Point) {bot + 1, 0});
        int32_t tail = rope_point_to_index(buffer->text, (Point) {bot + 2, 0});
        Rope* line = rope_substr(buffer->text, head, tail);
        textbuffer_edit(buffer, head, tail, NULL);

        int32_t dst = rope_point_to_index(buffer->text, (Point) {top, 0});
        textbuffer_edit(buffer, dst, dst, line);
        i--;
    }

    // Move lines up.
    while (i < 0) {
        int32_t top = rope_lines(buffer->text);
        int32_t bot = 0;
        for (int x = 0; x < buffer->selections->size; x++) {
            Selection* sel = buffer->selections->data[x];
            Point phead = rope_index_to_point(buffer->text, head(sel));
            Point ptail = rope_index_to_point(buffer->text, tail(sel));
            top = MIN(phead.row, top);
            bot = MAX(ptail.row, bot);
        }

        if (bot <= 0) break;

        int32_t head = rope_point_to_index(buffer->text, (Point) {top - 1, 0});
        int32_t tail = rope_point_to_index(buffer->text, (Point) {top, 0});
        Rope* line = rope_substr(buffer->text, head, tail);
        textbuffer_edit(buffer, head, tail, NULL);

        int32_t dst = rope_point_to_index(buffer->text, (Point) {bot, 0});
        textbuffer_edit(buffer, dst, dst, line);
        i++;
    }

    // Continuous Action.
    // No Action-End.
}

//
// Cursor Movement.
//

// - Cursor by Row/Column - //

void textbuffer_cursor_col (TextBuffer* buffer, int32_t i, bool s) {
    action_end(buffer);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        sel->cursor += i;
        if (sel->cursor < 0) sel->cursor = 0;
        if (sel->cursor > rope_len(buffer->text)) sel->cursor = rope_len(buffer->text);
        if (!s) sel->anchor = sel->cursor;
        sel->col_mem = rope_index_to_point(buffer->text, sel->cursor).col;
    }
}

void textbuffer_cursor_row (TextBuffer* buffer, int32_t i, bool s) {
    action_end(buffer);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        Point p = rope_index_to_point(buffer->text, sel->cursor);
        sel->cursor = rope_point_to_index(buffer->text, (Point) {p.row + i, sel->col_mem});
        if (!s) sel->anchor = sel->cursor;
    }
}

// - Cursor by Word Boundary - //

struct by_word_data{
    uint32_t chtype;
    uint32_t index;
};

static
bool by_word (uint32_t i, uint32_t ch, void* d) {
    struct by_word_data* data = d;
    data->index = i;
    if (chartype(ch) == data->chtype) {
        return true;
    }
    return false;
}
static
bool by_word_reverse (uint32_t i, uint32_t ch, void* d) {
    struct by_word_data* data = d;
    if (chartype(ch) == data->chtype) {
        data->index = i;
        return true;
    }
    return false;
}

void textbuffer_cursor_word (TextBuffer* buffer, int32_t i, bool s) {
    action_end(buffer);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        int32_t n = i;

        // Forwards.
        while (n > 0) {
            int32_t start = sel->cursor + 1;
            struct by_word_data data = {
                .chtype = chartype(rope_get_char(buffer->text, start)),
                .index = start,
            };
            rope_foreach_suffix(buffer->text, start, by_word, &data);
            sel->cursor = data.index;
            n--;
        }

        // Backwards.
        while (n < 0) {
            int32_t start = MAX(0, sel->cursor - 2);
            struct by_word_data data = {
                .chtype = chartype(rope_get_char(buffer->text, start)),
                .index = start,
            };
            rope_foreach_reverse_prefix(buffer->text, start, by_word_reverse, &data);
            sel->cursor = data.index;
            n++;
        }

        if (!s) sel->anchor = sel->cursor;
        sel->col_mem = rope_index_to_point(buffer->text, sel->cursor).col;
    }
}

// - Cursor by Line Boundary - //

void textbuffer_cursor_line (TextBuffer* buffer, int32_t i, bool s) {
    action_end(buffer);

    for (int x = 0; x < buffer->selections->size; x++) {
        Selection* sel = buffer->selections->data[x];
        int32_t n = i;

        // Forwards.
        while (n > 0) {
            int32_t start = sel->cursor + 1;
            Point p = rope_index_to_point(buffer->text, start);
            sel->cursor = MAX(0, rope_point_to_index(buffer->text, (Point) {p.row + 1, 0}) - 1);
            n--;
        }

        // Backwards.
        while (i < 0) {
            uint32_t start = MAX(0, sel->cursor - 1);
            Point p = rope_index_to_point(buffer->text, start);
            sel->cursor = rope_point_to_index(buffer->text, (Point) {p.row, 0});
            n++;
        }

        if (!s) sel->anchor = sel->cursor;
        sel->col_mem = rope_index_to_point(buffer->text, sel->cursor).col;
    }
}

// - Other Cursor Movement - //

void textbuffer_cursor_paragraph (TextBuffer* buffer, int32_t i, bool s) {

}

void textbuffer_cursor_goto (TextBuffer* buffer, int32_t row, int32_t col, bool s) {
    action_end(buffer);

    // Clear selection list except the primary selection.
    Selection* sel = selection_array_clear(buffer->selections);
    array_add(buffer->selections, sel);

    sel->cursor = rope_point_to_index(buffer->text, (Point){row, col});
    if (!s) sel->anchor = sel->cursor;
    sel->col_mem = rope_index_to_point(buffer->text, sel->cursor).col;
}

//
// Selection Operations.
//

// - Basic Operations - //

void textbuffer_selection_clear (TextBuffer* buffer) {
    action_end(buffer);

    if (selection_array_max_len(buffer->selections) > 0) {
        // Remove Selection Region from all cursors.
        for (int x = 0; x < buffer->selections->size; x++) {
            Selection* sel = buffer->selections->data[x];
            sel->anchor = sel->cursor;
        }
    } else {
        // Clear selection list except the primary selection.
        Selection* sel = selection_array_clear(buffer->selections);
        array_add(buffer->selections, sel);
    }
}

void textbuffer_selection_swap (TextBuffer* buffer) {

}

// - Search-for-Match Operations - //

void textbuffer_selection_next (TextBuffer* buffer, int32_t i) {

}

void textbuffer_selection_add_next (TextBuffer* buffer, int32_t i) {

}

// - Multi-Cursor Operations - //

void textbuffer_selection_add_next_row (TextBuffer* buffer, int32_t i) {
    action_end(buffer);

    while (i > 0) {
        Selection* sel = array_peek(buffer->selections);

        Point p = rope_index_to_point(buffer->text, sel->cursor);
        if (p.row >= rope_lines(buffer->text) - 1) break;
        int32_t index = rope_point_to_index(buffer->text, (Point){p.row + 1, p.col});

        Selection* new = selection_create();
        new->anchor = new->cursor = index;
        new->col_mem = sel->col_mem;
        array_add(buffer->selections, new);

        i--;
    }

    // Backwards.
    while (i < 0) {
        Selection* sel = buffer->selections->data[0];
        Point p = rope_index_to_point(buffer->text, sel->cursor);
        if (p.row <= 0) break;
        int32_t index = rope_point_to_index(buffer->text, (Point){p.row - 1, p.col});

        Selection* new = selection_create();
        new->anchor = new->cursor = index;
        new->col_mem = sel->col_mem;
        array_insert(buffer->selections, 0, new);

        i++;
    }
}

void textbuffer_selection_add_next_word (TextBuffer* buffer, int32_t i) {

}

void textbuffer_selection_add_next_line (TextBuffer* buffer, int32_t i) {

}
