#pragma once

#include "main.h"

struct box {
    uint32_t x, y, width, height;
};


void output_init ();
void output_fini ();

int output_char (int c);
void output_uchar (uint32_t u);
void output_str (const char* str);

void output_frame ();
void output_reset ();
void output_clear ();

void output_cup (int32_t row, int32_t col);

void output_normal ();
void output_setfg (int32_t fg);
void output_setbg (int32_t bg);
void output_bold ();
void output_reverse ();
void output_underline ();
void output_no_underline ();

void output_cnorm ();
void output_civis ();
void output_cvvis ();
