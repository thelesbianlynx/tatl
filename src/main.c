#include "main.h"

#include <sys/ioctl.h>
#undef CTRL

#include "array.h"
#include "input.h"
#include "output.h"
#include "editor.h"

#include "colorizer.h"

bool cursor_blink = false;
State* test_state;

int main (int argc, char** argv) {
    // Initialize Test State.
    test_state = state_create();
    state_append(test_state, "//", STYLE_COMMENT);

    state_append(test_state, "int", STYLE_KEYWORD); 
    state_append(test_state, "long", STYLE_KEYWORD);
    state_append(test_state, "short", STYLE_KEYWORD);
    state_append(test_state, "float", STYLE_KEYWORD);
    state_append(test_state, "double", STYLE_KEYWORD);
    state_append(test_state, "bool", STYLE_KEYWORD);
    state_append(test_state, "char", STYLE_KEYWORD);
    state_append(test_state, "signed", STYLE_KEYWORD);
    state_append(test_state, "unsigned", STYLE_KEYWORD);
    state_append(test_state, "true", STYLE_KEYWORD);
    state_append(test_state, "false", STYLE_KEYWORD);
    state_append(test_state, "void", STYLE_KEYWORD);
    state_append(test_state, "if", STYLE_KEYWORD);
    state_append(test_state, "else", STYLE_KEYWORD);
    state_append(test_state, "switch", STYLE_KEYWORD);
    state_append(test_state, "case", STYLE_KEYWORD);
    state_append(test_state, "default", STYLE_KEYWORD);
    state_append(test_state, "do", STYLE_KEYWORD);
    state_append(test_state, "for", STYLE_KEYWORD);
    state_append(test_state, "while", STYLE_KEYWORD);
    state_append(test_state, "break", STYLE_KEYWORD);
    state_append(test_state, "continue", STYLE_KEYWORD);
    state_append(test_state, "return", STYLE_KEYWORD);
    state_append(test_state, "goto", STYLE_KEYWORD);
    state_append(test_state, "auto", STYLE_KEYWORD);
    state_append(test_state, "register", STYLE_KEYWORD);
    state_append(test_state, "static", STYLE_KEYWORD);
    state_append(test_state, "extern", STYLE_KEYWORD);
    state_append(test_state, "const", STYLE_KEYWORD);
    state_append(test_state, "volitile", STYLE_KEYWORD);
    state_append(test_state, "sizeof", STYLE_KEYWORD);
    state_append(test_state, "struct", STYLE_KEYWORD);
    state_append(test_state, "union", STYLE_KEYWORD);
    state_append(test_state, "enum", STYLE_KEYWORD);
    state_append(test_state, "typedef", STYLE_KEYWORD);
    state_append(test_state, "inline", STYLE_KEYWORD);
    state_append(test_state, "restrict", STYLE_KEYWORD);

    state_append(test_state, "(", STYLE_SYMBOL);
    state_append(test_state, ")", STYLE_SYMBOL);
    state_append(test_state, "[", STYLE_SYMBOL);
    state_append(test_state, "]", STYLE_SYMBOL);
    state_append(test_state, "{", STYLE_SYMBOL);
    state_append(test_state, "}", STYLE_SYMBOL);
    state_append(test_state, ";", STYLE_SYMBOL);
    state_append(test_state, ":", STYLE_SYMBOL);
    state_append(test_state, ".", STYLE_SYMBOL);
    state_append(test_state, ",", STYLE_SYMBOL);
    state_append(test_state, "*", STYLE_SYMBOL);
    state_append(test_state, "->", STYLE_SYMBOL);

    //
    // Process Arguments
    //
    Array* filenames = array_create();
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Handle Option.
            continue;
        }

        // Add file to list.
        array_add(filenames, argv[i]);
    }

    /*
    Array* files = array_create();
    char* dir = getcwd(NULL,0);

    search_load_files(files, dir);
    search_rank_files(files, "rope");

    printf("=======\n");
    for (int i = 0; i < files->size; i++) {
        FileEntry* file = files->data[i];
        printf("%s\n", file->path->buffer);
    }

    search_unload_files(files);
    free(dir);
    array_destroy(files);

    return 0;
    */
    
    //
    // Launch Editor.
    //
    output_init();

    Editor editor;
    editor_init(&editor, filenames);

    InputEvent event = {};

    struct winsize size;
    int width = 0, height = 0;

    bool exit = false;
    while (!exit) {
        int32_t debug[32] = {0};
        bool has_event = nextkey(10, &event, debug);
        if (has_event) {
            exit = !editor_event(&editor, &event);
            cursor_blink = false;
        } else {
            cursor_blink = !cursor_blink;
        }

        if (ioctl(0, TIOCGWINSZ, &size) == 0) {
            width = size.ws_col;
            height = size.ws_row;
        }


        Box window = {0, 0, width, height};
        output_clear();
        editor_draw(&editor, &window, event.type == INPUT_MOUSE ? &event.m_event : NULL);
        output_frame();
    }

    array_destroy(filenames);

    editor_fini(&editor);

    output_cnorm();
    output_fini();

    state_destroy(test_state);
}
