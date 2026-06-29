#include "colorizer.h"

#include "array.h"
#include "character.h"


//
// Colorizer States.
//

State* state_create () {
    State* state = malloc(sizeof(State));
    state->ch = 0;
    state->type = 0;
    state->terminal = false;
    state->next_state = array_create();
    return state;
}

void state_destroy (State* state) {
    for (int i = 0; i < state->next_state->size; i++) {
        state_destroy(state->next_state->data[i]);
    }
    array_destroy(state->next_state);
    free(state);
}

void state_append (State* state, const char* str, uint32_t type) {
    assert(str != NULL && *str != 0 && "Invalid String into state_append");
    int32_t ch;
    while((ch = (int32_t) *str++) != 0) {
        State* next = NULL;
        for (int i = 0; i < state->next_state->size; i++) {
            State* nx = state->next_state->data[i];
            if (nx->ch == ch) { 
                next = nx;
                break;
            }
        }

        if (next == NULL) {
            next = state_create();
            next->ch = ch;
            array_add(state->next_state, next);
        }

        state = next;
    }

    assert(state->terminal == false && "Duplicate state_append String");
    state->type = type;
    state->terminal = true;
}

static 
void state_print (State* state, int lvl) {
    // Indentation level.
    for (int i = 0; i < lvl; i++) printf("  ");

    printf("[%c: %d %s]\n", (char) state->ch, state->type, state->terminal ? "true" : "false");
    for (int i = 0; i < state->next_state->size; i++) {
        state_print(state->next_state->data[i], lvl + 1);
    }
}

static
State* next_state (State* current, int32_t ch) {
    for (int i = 0; i < current->next_state->size; i++) {
        State* next = current->next_state->data[i];
        if (next->ch == ch) {
            return next;
        } 
    }

    return NULL;
}


//
// Colorizer Logic.
//

void colorize_begin_line (Colorizer* colorizer) {
    colorizer->comment = false;
    colorizer->col_last = 0;
    colorizer->state_current = colorizer->state_start;
}

static
void apply_color (int32_t color_mask, uint32_t col, uint32_t col_last, uint32_t col_start, uint32_t col_end, int32_t* style) {
    for (int c = col_last; c < col; c++) {
        if (c < col_end && c >= col_start) {
            style[c] |= color_mask;
        }
    }
}


// NOTE: col represents the column immediatly after the emited char, not the column of the char itself
//  (This is because of tabs, see textview.c) 
void colorize_next_char (Colorizer* colorizer, int32_t ch, uint32_t col, uint32_t col_start, uint32_t col_end, int32_t* style) {
    // Special case: line comment.
    if (colorizer->comment) {
        apply_color(STYLE_COMMENT, col, colorizer->col_last, col_start, col_end, style);
        colorizer->col_last = col;
        return;
    }

    State* state = colorizer->state_current;
    State* next = next_state(state, ch);
    
    if (next == NULL) {
        //if (state->type == STATE_KEYWORD && state->terminal && chartype(ch) != CHARTYPE_TEXT) {
        //    apply_color(color, col, colorizer->col_last, col_start, col_end, style);
        //    colorizer->col_last = col;
        //}

        // col_last is no greater than (col - 1) so this will do nothing in that case.
        apply_color(0, col - 1, colorizer->col_last, col_start, col_end, style);
        colorizer->col_last = col - 1;
        
        // Try from start state.
        next = next_state(colorizer->state_start, ch);
    } 

    if (next != NULL) {
        if (next->terminal) {
            if (next->type == STYLE_COMMENT) {
                colorizer->comment = true;
            }
            apply_color(next->type, col, colorizer->col_last, col_start, col_end, style);
            colorizer->col_last = col;
        }
        colorizer->state_current = next;
    } else {
        colorizer->state_current = colorizer->state_start;
        colorizer->col_last = col;
    }

    /* if (next != NULL) {
        switch (next->type) {
            case STATE_LINE_COMMENT:
                color |= STYLE_COMMENT;
                break;
            case STATE_BEGIN_COMMENT:
                color |= STYLE_COMMENT;
                colorizer->clevel = 1;
                break;
            case STATE_END_COMMENT:
                color |= STYLE_COMMENT;
                colorizer->clevel = 0; // No Nesting comments.
                break;
            case STATE_SYMBOL:
                color |= STYLE_SYMBOL;
                break;
            
        }

        //colorizer->state_current = next;
    }

    apply_color(color, col, colorizer->col_last, col_start, col_end, style);
    colorizer->col_last = col;
    

     if (col->state == 0) {
        if (ch == '/') {
            col->cstate++;
            if (col->cstate == 2) {
                style[*i] |= 8;
                style[*i+1] |= 8;
                (*i) += 2;
                col->state = -1;
            }
        } else {
            (*i) += col->cstate;
        
            switch (ch) {
                case '{':
                case '}':
                case '[':
                case ']':
                case '(':
                case ')':
                case ';':
                case ':':
                case '.':
                case ',': {
                    style[*i] |= 4;
                    break;
                }
            }
            (*i)++;
        }
    } else if (col->state == -1) {
        style[*i] |= 8;
        (*i)++;
    } */
}
