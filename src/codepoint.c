#include "codepoint.h"


int32_t codepoint_to_chars (char* buffer, uint32_t code) {
    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }

    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);            /* 110xxxxx */
        buffer[1] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 2;
    }

    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);           /* 1110xxxx */
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[2] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 3;
    }

    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);           /* 11110xxx */
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);  /* 10xxxxxx */
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[3] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 4;
    }

    return 0;
}


int32_t chars_to_codepoint (char* buffer, uint32_t len, uint32_t* code) {
    if ((unsigned char) buffer[0] <= 127) {
        //printf("%x\n", (int) buffer[0]);
        *code = buffer[0];
        return 1;
    }

    if ((buffer[0] & 0xF0) == 0xF0) {
        if (len < 4) return 0;
        *code = (((unsigned char) buffer[0] & ~0xF0) << 18)
            |   (((unsigned char) buffer[1] & ~0x80) << 12)
            |   (((unsigned char) buffer[2] & ~0x80) << 6)
            |    ((unsigned char) buffer[3] & ~0x80);
        return 4;
    }

    if ((buffer[0] & 0xE0) == 0xE0) {
        if (len < 3) return 0;
        *code = (((unsigned char) buffer[0] & ~0xE0) << 12)
            |   (((unsigned char) buffer[1] & ~0x80) << 6)
            |    ((unsigned char) buffer[2] & ~0x80);
        return 3;
    }

    if ((buffer[0] & 0xC0) == 0xC0) {
        if (len < 2) return 0;
        *code = (((unsigned char) buffer[0] ^ 0xC0) << 6)
            |    ((unsigned char) buffer[1] ^ 0x80);
        return 2;
    }

    return 0;
}
