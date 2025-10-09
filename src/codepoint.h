#pragma once

#include "main.h"

int32_t codepoint_to_chars (char* buffer, uint32_t code);

int32_t chars_to_codepoint (char* buffer, uint32_t len, uint32_t* code);
