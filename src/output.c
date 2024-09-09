#include "output.h"

#include <curses.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>


static struct termios term_save;

static uint32_t buf_size;
static uint32_t buf_capacity;

static char* buffer;


void output_init () {
    buf_size = 0;
    buf_capacity = 65536;
    buffer = malloc(buf_capacity);

    setupterm(NULL, 1, NULL);
    tcgetattr(0, &term_save);
    {
        struct termios term_raw = term_save;
        cfmakeraw(&term_raw);
        tcsetattr(0, TCSANOW, &term_raw);
    }

    tputs(tigetstr("smcup"), 1, output_char);
    output_str("\33[?1002h");
    output_frame();
}

void output_fini () {
    tputs(tigetstr("rmcup"), 1, output_char);
    output_str("\33[?1002l");
    output_frame();
    tcsetattr(0, TCSANOW, &term_save);
    printf("%d\n", buf_capacity);
}

static void expand () {
    buf_capacity *= 2;
    char* nbuffer = malloc(buf_capacity);

    for (int i = 0; i < buf_size; ++i){
        nbuffer[i] = buffer[i];
    }

    free(buffer);
    buffer = nbuffer;
}

int output_char (int c) {
    if (buf_size == buf_capacity) {
       expand();
    }
    buffer[buf_size] = (char) c;
    buf_size++;
    return c;
}

void output_uchar (uint32_t u) {
    // Not Final.
    output_char((char) u);
}


void output_str (const char* str) {
    for (;;) {
        char c = *(str++);
        if (c == '\0') return;
        output_char(c);
    }
}


void output_frame () {
    write(1, buffer, buf_size);
    buf_size = 0;
}
void output_reset () {
    buf_size = 0;
}


void output_cup (int32_t row, int32_t col) {
    tputs(tparm(tigetstr("cup"), row, col), 1, output_char);
}

void output_setfg (int32_t fg) {
    tputs(tparm(tigetstr("setaf"), fg), 1, output_char);
}

void output_setbg (int32_t bg) {
    tputs(tparm(tigetstr("setab"), bg), 1, output_char);
}

void output_clear () {
    tputs(tparm(tigetstr("clear")), 1, output_char);
}

void output_cnorm () {
    tputs(tparm(tigetstr("cnorm")), 1, output_char);
}

void output_civis () {
    tputs(tparm(tigetstr("civis")), 1, output_char);
}

void output_cvvis () {
    tputs(tparm(tigetstr("cvvis")), 1, output_char);
}

void output_underline () {
    tputs(tparm(tigetstr("smul")), 1, output_char);
}

void output_no_underline () {
    tputs(tparm(tigetstr("rmul")), 1, output_char);
}
