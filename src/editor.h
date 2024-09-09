#pragma once

#include "main.h"

struct editor {
    int32_t width, height;

    Buffer* buffer;

    bool blink;

    int32_t debug;

    uint32_t mstate;
    uint32_t mx;
    uint32_t my;
};


void editor_init (Editor* editor);
void editor_fini (Editor* editor);

bool editor_update (Editor* editor, InputStatus status, InputState* state);
void editor_draw (Editor* editor, int32_t width, int32_t height, int32_t* debug);
