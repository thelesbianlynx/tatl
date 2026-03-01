#pragma once

#include "main.h"

#include "textbuffer.h"
#include "textview.h"
#include "textaction.h"


struct filebuffer {

    TextBuffer* buffer;
    TextView* view;

    CharBuffer* title;
    CharBuffer* longpath;
    CharBuffer* shortpath;

};


FileBuffer* filebuffer_create ();

void filebuffer_destroy (FileBuffer* fb);


void filebuffer_read (FileBuffer* fb, const char* path);

void filebuffer_write (FileBuffer* fb, const char* path);


void filebuffer_draw (FileBuffer* fb, Box* window, MouseEvent* mev);
