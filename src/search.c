#include "search.h"

#include <dirent.h>

#include "array.h"
#include "charbuffer.h"
#include "output.h"


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
    search->selection = 0;
    search->scroll = 0;
    search->scroll_dmg = false;
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


//static
//bool filter_file (const char* filename) {
//    int len = strlen(filename);
//
//    if (len >= 2) {
//        // Ignore object (.o) files.
//        if (filename[len-1] == 'o' && filename[len - 2] == '.') return false;
//        // Ignore static library (.a) files.
//        if (filename[len-1] == 'a' && filename[len - 2] == '.') return false;
//        // Ignore dependency (.d) files.
//        if (filename[len-1] == 'd' && filename[len - 2] == '.') return false;
//    }
//
//    if (len >= 3) {
//        // Ignore shared object (.so) files.
//        if (filename[len-1] == 'o' && filename[len - 2] == 's'
//            && filename[len - 3] == '.') return false;
//    }
//
//    return true;
//}

static
int get_files (Search* search, FileEntry* parent, const char* path, uint32_t prefix) {
    struct dirent** entry_list;
    int n = scandir(path, &entry_list, NULL, alphasort);
    if (n < 0) return 0;

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
                // Need to check if we expanded an empty directory.
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
    return n;
}

void search_load_files (Search* search) {
    entry_list_clear(search->files);
    get_files(search, NULL, search->path->buffer, search->path->size);

    search->selection = 0;
    search->scroll_dmg = true;
}

void search_unload_files (Search* search) {
    entry_list_clear(search->files);
}


//
// Entry Navigation.
//

FileEntry* search_get_entry (Search* search) {
    if (search->files->size == 0) return NULL;
    if (search->selection >= search->files->size) search->selection = search->files->size - 1;
    if (search->selection < 0) search->selection = 0;
    return search->files->data[search->selection];
}

void search_next (Search* search, int32_t i) {
    if (search->files->size == 0) return;
    search->selection = MOD(search->selection + 1, search->files->size);
    search->scroll_dmg = true;
}

void search_prev (Search* search, int32_t i) {
    if (search->files->size == 0) return;
    search->selection = MOD(search->selection - 1, search->files->size);
    search->scroll_dmg = true;
}


//
// Expand/Collapse directories.
//

void search_expand (Search* search) {
    FileEntry* entry = search_get_entry(search);
    if (entry == NULL) return;

    if (entry->is_dir) {
        // Expand Directory.
        array_remove_item(search->files, entry);
        array_add(search->dirs, entry);
        search_load_files(search); // This is not the best way to do this but it is easiest.
        search->selection = entry->pos;
        search->scroll_dmg = true;
    } else {
        // Collapse Directory
        search_collapse(search);
    }
}


void search_collapse (Search* search) {
    FileEntry* entry = search_get_entry(search);
    if (entry == NULL) return;

    // Collapse Directory
    FileEntry* parent = entry->parent;
    if (parent == NULL) return;

    array_remove_item(search->dirs, parent);
    uint64_t scroll_ino = parent->ino;
    entry_destroy(parent);
    search_load_files(search);

    // Restore Scroll position.
    for (int i = 0; i < search->files->size; i++) {
        FileEntry* e = search->files->data[i];
        if (e->ino == scroll_ino) {
            search->selection = e->pos;
            search->scroll_dmg = true;
        }
    }
}


//
// Directory Navigation.
//

void search_forward (Search* search) {
    FileEntry* entry = search_get_entry(search);
    if (entry == NULL) return;
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

    search->selection = 0;
    search->scroll_dmg = true;
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

    search->selection = 0;
    search->scroll_dmg = true;
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

//
// Draw Search Window.
//

void search_draw (Search* search, Box* window, MouseEvent* mev) {
    // Mouse Input.
    if (mev != NULL) {

    }

    // Scroll damage.
    if (search->scroll_dmg) {
        if (search->selection < search->scroll) {
            search->scroll = search->selection;
        }
        if (search->selection > search->scroll + window->height - 2) {
            search->scroll = MAX(0, search->selection - window->height + 2);
        }

        search->scroll_dmg = false;
    }

    // Search Location.
    {
        output_cup(window->y, window->x);
        output_setfg(COLOR_ACCENT);
        output_reverse();
        char msg[window->width + 1], buf[window->width + 1];
        snprintf(msg, window->width + 1, " Index of: %s ", search->path->buffer);
        snprintf(buf, window->width + 1, "%-*s", window->width, msg);
        output_str(buf);
        output_normal();
    }

    // Search entries.
    for (int i = 0; i < window->height - 1; i++) {
        int32_t n = i + search->scroll;
        if (n >= search->files->size) break;

        FileEntry* file = search->files->data[n];

        char buf[window->width + 1];
        if (file->is_dir) {
            snprintf(buf, window->width + 1 ," * %s/ ", file->path->buffer + file->prefix + 1);
        } else {
            snprintf(buf, window->width + 1 ," * %s ", file->path->buffer + file->prefix + 1);
        }

        output_cup(window->y + i + 1, window->x);
        if (n == search->selection) {
            output_setbg(COLOR_HIGHLIGHT);
            output_str(buf);
            output_normal();
        } else {
            output_str(buf);
        }
    }
}

