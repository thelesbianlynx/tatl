#pragma once

#include "main.h"

//
// Character Type Enum.
enum {
    CHARTYPE_WS,
    CHARTYPE_TEXT,
    CHARTYPE_SYMBOL,
};

uint32_t chartype (int32_t ch);


int32_t codepoint_to_chars (char* buffer, uint32_t code);

int32_t chars_to_codepoint (char* buffer, uint32_t len, uint32_t* code);
