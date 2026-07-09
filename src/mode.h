#pragma once

#include "main.h"


struct mode {
    const char* name;
    State* colorizer_state;
    bool force_hard_tabs;
};


Mode* get_language_mode (const char* filename);
