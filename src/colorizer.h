#pragma once

#include "main.h"

enum {
    STYLE_SELECTION = 1,
    STYLE_CURSOR = 2,
    STYLE_SYMBOL = 4,
    STYLE_COMMENT = 8,
    STYLE_KEYWORD = 16,
    STYLE_NAME = 32,
    STYLE_STRING = 64,
    STYLE_CHAR = 128,
};

enum {
    STATE_NORMAL = 0,
    STATE_LINE_COMMENT,
    STATE_BEGIN_COMMENT,
    STATE_END_COMMENT,
    STATE_PUSH_COMMENT,
    STATE_POP_COMMENT,
    STATE_STRING,
    STATE_CHAR,
    STATE_KEYWORD,
    STATE_SYMBOL,
};


struct state {
    int32_t ch;
    uint32_t type;
    uint32_t depth;
    bool terminal;

    Array* next_state;
};

struct colorizer {
    State* state_start;
    State* state_current;

    uint32_t col_last;
    int32_t string_type;
    int32_t comment_depth;
    bool line_comment;
    bool saw_slash;
};

State* state_create ();

void state_destroy (State* state);


void state_append (State* state, const char* str, uint32_t type);

void state_print (State* state, uint32_t lvl);


void colorize_begin_line (Colorizer* colorizer, int32_t comment_depth);

void colorize_next_char (Colorizer* colorizer, int32_t ch, uint32_t col, uint32_t col_start, uint32_t col_end, int32_t* style);

void colorize_next_char_fast (Colorizer* col, int32_t ch);
