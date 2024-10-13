#pragma once

#include "main.h"

enum {
    MODE_NORMAL,
    MODE_EDIT,
};

struct editor {
    Buffer* buffer;

    uint32_t mode;

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

void editor_enter_mode (Editor* editor, uint32_t mode);
void editor_exit_mode (Editor* editor);
