#pragma once

#include "main.h"


struct file_entry {
    CharBuffer* path;

    uint32_t pos;
    uint32_t rank;
    uint32_t prefix;
    uint32_t title;
};

void search_load_files (Array* files, const char* path);

void search_unload_files (Array* files);


void search_rank_files (Array* files, const char* prompt);
