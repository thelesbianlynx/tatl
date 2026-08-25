#pragma once

#include "main.h"


struct search {
    CharBuffer* path;
    Array* files;
    Array* dirs;
    uint32_t flags;
};

struct file_entry {
    CharBuffer* path;
    FileEntry* parent;

    uint64_t ino;
    uint32_t pos;
    uint32_t rank;
    uint32_t prefix;
    uint32_t title;
    bool is_dir;
    bool mark;
};


Search* search_create ();

void search_destroy (Search* search);


void search_set_directory (Search* search, const char* path);


void search_load_files (Search* search);

void search_unload_files (Search* search);


void search_expand (Search* search, uint32_t entry);


void search_forward (Search* search, uint32_t entry);

void search_backward (Search* search);


void search_rank_files (Search* search, const char* prompt);
