#pragma once

#include "main.h"
#include "input.h"

struct editor {
    uint32_t width, height;

    Buffer* buffer;
};


void editor_init (Editor* editor);
void editor_fini (Editor* editor);

bool editor_update (Editor* editor, InputStatus status, InputState* state);
void editor_draw (Editor* editor, uint32_t width, uint32_t height, uint32_t* debug);
