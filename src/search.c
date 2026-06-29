#include "search.h"

#include <dirent.h>

#include "array.h"
#include "charbuffer.h"


//
// Search for files recursively in a directory.
//


static
bool filter_file (const char* filename) {
    CharBuffer* buf = charbuffer_create();
    charbuffer_astr(buf, filename);
    bool r = true;

    if (buf->size >= 2) {
        // Ignore object (.o) files.
        if (buf->buffer[buf->size-1] == 'o' && buf->buffer[buf->size - 2] == '.') r = false;
        // Ignore static library (.a) files.
        if (buf->buffer[buf->size-1] == 'a' && buf->buffer[buf->size - 2] == '.') r = false;
    }

    if (buf->size >= 3) {
        // Ignore shared object (.so) files.
        if (buf->buffer[buf->size-1] == 'o' && buf->buffer[buf->size - 2] == 's'
            && buf->buffer[buf->size - 3] == '.') r = false;
    }

    charbuffer_destroy(buf);
    return r;
}

static
void get_files (Array* files, const char* path, uint32_t prefix) {
    struct dirent** entry_list;
    int n = scandir(path, &entry_list, NULL, alphasort);
    if (n < 0) return;

    for (int i = 0; i < n; i++) {
        struct dirent* entry = entry_list[i];
        if (*entry->d_name == '.') {
            free(entry);
            continue;
        }
        if (!filter_file(entry->d_name)) {
            free(entry);
            continue;
        }

        CharBuffer* subdir = charbuffer_create();
        charbuffer_astr(subdir, path);
        int len = subdir->size;

        if (subdir->size >= 1 && subdir->buffer[subdir->size - 1] != '/') {
            charbuffer_achar(subdir, '/');
        }
        charbuffer_astr(subdir, entry->d_name);

        if (entry->d_type == DT_DIR) {
            get_files(files, subdir->buffer, prefix);
            charbuffer_destroy(subdir);
        } else if (entry->d_type == DT_REG) {
            FileEntry* file = malloc(sizeof(FileEntry));
            file->path = subdir;
            file->pos = files->size;
            file->rank = 0;
            file->prefix = prefix;
            file->title = len;
            array_add(files, file);
        }

        free(entry);
    }

    free(entry_list);
}

void search_load_files (Array* files, const char* path) {
    get_files(files, path, strlen(path));
}

void search_unload_files (Array* files) {
    for (int i = 0; i < files->size; i++) {
        FileEntry* file = files->data[i];
        charbuffer_destroy(file->path);
        free(file);
    }
    array_clear(files);
}


//
// Rank files relative to a search query.
//

static
uint32_t rank_file (FileEntry* file, const char* query) {
    int x = 0;
    uint32_t rank = 0;
    for (int i = file->title; i < file->path->size; i++) {
        char ch = query[x];
        if (ch == '\0') return rank;

        if (file->path->buffer[i] == ch) {
            rank++;
            x++;
        }
    }
    return rank;
}

static
int filecmp (const void* f1, const void* f2) {
    const FileEntry* file1 = *(FileEntry**) f1;
    const FileEntry* file2 = *(FileEntry**) f2;

    if (file1->rank == file2->rank) {
        if (file1->pos == file2->pos) return  0;
        if (file1->pos > file2->pos)  return  1;
        if (file1->pos < file2->pos)  return -1;
    }
    if (file1->rank > file2->rank)    return -1;
    if (file1->rank < file2->rank)    return  1;

    return 0; // to make gcc happy.
}

void search_rank_files (Array* files, const char* query) {
    for (int i = 0; i < files->size; i++) {
        FileEntry* file = files->data[i];
        file->rank = rank_file(file, query);
    }

    qsort(files->data, files->size, sizeof(void*), filecmp);
}
