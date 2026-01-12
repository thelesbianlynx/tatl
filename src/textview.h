#pragma once

#include "main.h"


struct textview {
    TextBuffer* buffer;

    int32_t scroll_line;
    int32_t scroll_col;
};

TextView* textview_create (TextBuffer* buffer);

void textview_destroy (TextView* view);


void textview_draw (TextView* view, Box* box, MouseEvent* mev);
