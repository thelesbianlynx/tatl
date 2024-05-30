#pragma once

#include "main.h"
#include "input.h"

struct editor {
    int32_t width, height;

    Buffer* buffer;
};


void editor_init (Editor* editor);
void editor_fini (Editor* editor);

bool editor_update (Editor* editor, InputStatus status, InputState* state);
void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug);
