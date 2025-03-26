#pragma once

//
// Common Standard Includes.
//
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

typedef struct input_event InputEvent;

typedef struct box Box;

typedef struct point Point;
typedef struct buffer Buffer;

typedef struct array Array;
typedef struct charbuffer CharBuffer;


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
