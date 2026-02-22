#pragma once

//
// Common Standard Includes.
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <math.h>

//
// Type Listing.
//
typedef struct editor Editor;

typedef struct rope Rope;
typedef struct point Point;

typedef struct textbuffer TextBuffer;
typedef struct selection Selection;

typedef struct textview TextView;

typedef struct array Array;
typedef struct charbuffer CharBuffer;
typedef struct intbuffer IntBuffer;

typedef struct input_event InputEvent;
typedef struct mouse_event MouseEvent;

typedef struct box Box;


//
// Macros
//
static inline int32_t MOD (int32_t a, int32_t m) {
    return ((a % m) + m) % m;
}

static inline int32_t MIN (int32_t a, int32_t b) {
    return a < b ? a : b;
}

static inline int32_t MAX (int32_t a, int32_t b) {
    return a > b ? a : b;
}

static inline int32_t ABS (int32_t a) {
    return a < 0 ? -a : a;
}

//
// Defines.
//
#define NODE_CONTENT_SIZE 128
#define HIST_LIMIT 1024
