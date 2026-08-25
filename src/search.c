#include "search.h"

#include <dirent.h>

#include "array.h"
#include "charbuffer.h"


//
// Functions for dealing with file entries.
//

static
FileEntry* entry_create (CharBuffer* path, FileEntry* parent, uint64_t ino, uint32_t pos, uint32_t prefix, uint32_t title, bool is_dir) {
    FileEntry* entry = malloc(sizeof(FileEntry));
    entry->path = path;
    entry->parent = parent;
    entry->ino = ino;
    entry->pos = pos;
    entry->rank = 0;
    entry->prefix = prefix;
    entry->title = title;
    entry->is_dir = is_dir;
    entry->mark = false;
    return entry;
}

static
void entry_destroy (FileEntry* entry) {
    charbuffer_destroy(entry->path);
    free(entry);
}

static
void entry_list_clear (Array* entries) {
    for (int i = 0; i < entries->size; i++) {
        entry_destroy(entries->data[i]);
    }
    array_clear(entries);
}

static
void entry_list_mark (Array* entries) {
    for (int i = 0; i < entries->size; i++) {
        FileEntry* entry = entries->data[i];
        assert(entry->is_dir);
        entry->mark = true;
    }
}

static
FileEntry* entry_list_get_by_ino (Array* entries, uint64_t ino) {
    for (int i = 0; i < entries->size; i++) {
        FileEntry* entry = entries->data[i];
        if (entry->ino == ino) return entry;
    }
    return NULL;
}


//
// Search Object.
//

Search* search_create () {
    Search* search = malloc(sizeof(Search));
    search->path = charbuffer_create();
    search->files = array_create();
    search->dirs = array_create();
    search->flags = 0;
    return search;
}

void search_destroy (Search* search) {
    entry_list_clear(search->files);
    entry_list_clear(search->dirs);
    charbuffer_destroy(search->path);
    array_destroy(search->files);
    array_destroy(search->dirs);
    free(search);
}


void search_set_directory (Search* search, const char* path) {
    charbuffer_clear(search->path);
    charbuffer_astr(search->path, path);
    entry_list_clear(search->files);
    entry_list_clear(search->dirs);
}


//
// Search for files recursively in a directory.
//


static
bool filter_file (const char* filename) {
    int len = strlen(filename);

    if (len >= 2) {
        // Ignore object (.o) files.
        if (filename[len-1] == 'o' && filename[len - 2] == '.') return false;
        // Ignore static library (.a) files.
        if (filename[len-1] == 'a' && filename[len - 2] == '.') return false;
        // Ignore dependency (.d) files.
        if (filename[len-1] == 'd' && filename[len - 2] == '.') return false;
    }

    if (len >= 3) {
        // Ignore shared object (.so) files.
        if (filename[len-1] == 'o' && filename[len - 2] == 's'
            && filename[len - 3] == '.') return false;
    }

    return true;
}

static
void get_files (Search* search, FileEntry* parent, const char* path, uint32_t prefix) {
    struct dirent** entry_list;
    int n = scandir(path, &entry_list, NULL, alphasort);
    if (n < 0) return;

    for (int i = 0; i < n; i++) {
        struct dirent* entry = entry_list[i];
        if (*entry->d_name == '.') {
            free(entry);
            continue;
        }
        //if (!filter_file(entry->d_name)) {
        //    free(entry);
        //    continue;
        //}

        // Construct Entry's Full Path.
        CharBuffer* entry_path = charbuffer_create();
        charbuffer_astr(entry_path, path);
        if (entry_path->size >= 1 && entry_path->buffer[entry_path->size - 1] != '/')
            charbuffer_achar(entry_path, '/');
        int title = entry_path->size; // Start of file name.
        charbuffer_astr(entry_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Check directory list to see if this directory has been expanded.
            FileEntry* dir = entry_list_get_by_ino(search->dirs, entry->d_fileno);
            if (dir != NULL) {
                // Is Expanded, recursively search files.
                get_files(search, dir, entry_path->buffer, prefix);
                charbuffer_destroy(entry_path);
            } else {
                // Is not expanded, add file entry flaged as directory.
                FileEntry* file = entry_create(entry_path, parent, entry->d_fileno, search->files->size, prefix, title, true);
                array_add(search->files, file);
            }
        } else if (entry->d_type == DT_REG) {
            FileEntry* file = entry_create(entry_path, parent, entry->d_fileno, search->files->size, prefix, title, false);
            array_add(search->files, file);
        }

        free(entry);
    }

    free(entry_list);
}

void search_load_files (Search* search) {
    entry_list_clear(search->files);
    get_files(search, NULL, search->path->buffer, search->path->size);
}

void search_unload_files (Search* search) {
    entry_list_clear(search->files);
}


//
// Expand/Collapse directories.
//

void search_expand (Search* search, uint32_t entry_no) {
    assert(entry_no < search->files->size);
    FileEntry* entry = search->files->data[entry_no];
    if (entry->is_dir) {
        // Expand Directory.
        array_remove(search->files, entry_no);
        array_add(search->dirs, entry);
        search_load_files(search); // This is not the best way to do this but it is easiest.
    } else {
        // Collapse Directory
        FileEntry* parent = entry->parent;
        if (parent != NULL) {
            array_remove_item(search->dirs, parent);
            search_load_files(search); // Gonna need some way of resetting selected entry index.
        }
    }
}


//
// Directory Navigation.
//

void search_forward (Search* search, uint32_t entry_no) {
    assert(entry_no < search->files->size);
    FileEntry* entry = search->files->data[entry_no];
    if (entry->is_dir) {
        search_set_directory(search, entry->path->buffer);
        search_load_files(search);
    } else {
        FileEntry* parent = entry->parent;
        if (parent != NULL) {
            search_set_directory(search, parent->path->buffer);
            search_load_files(search);
        }
    }
}

void search_backward (Search* search) {
    int s = 0;
    for (int i = 0; i < search->path->size; i++) {
        if (search->path->buffer[i] == '/') {
            s = i;
        }
    }
    charbuffer_rm_suffix(search->path, s);
    search_load_files(search);
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

void search_rank_files (Search* search, const char* query) {
    for (int i = 0; i < search->files->size; i++) {
        FileEntry* file = search->files->data[i];
        file->rank = rank_file(file, query);
    }

    qsort(search->files->data, search->files->size, sizeof(void*), filecmp);
}
