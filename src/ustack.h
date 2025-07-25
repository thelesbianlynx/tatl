#pragma once

#include "main.h"


struct ustack {
    Array* undo;
    Array* redo;
};


UStack* ustack_create ();

void ustack_destroy (UStack* ustack);

void ustack_clear (UStack* ustack);

void ustack_push (UStack* ustack, Array* lines, Point* pre_cursor, Point* post_cursor);

bool ustack_undo (UStack* ustack, Array* lines, Point* cursor);

bool ustack_redo (UStack* ustack, Array* lines, Point* cursor);
