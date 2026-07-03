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
    state_append(test_state, "//", STATE_LINE_COMMENT);
    state_append(test_state, "/*", STATE_BEGIN_COMMENT);
    state_append(test_state, "*/", STATE_END_COMMENT);
    state_append(test_state, "\"", STATE_STRING);
    state_append(test_state, "'",  STATE_CHAR);

/*
    state_append(test_state, "int", STATE_KEYWORD); 
    state_append(test_state, "long", STATE_KEYWORD);
    state_append(test_state, "short", STATE_KEYWORD);
    state_append(test_state, "float", STATE_KEYWORD);
    state_append(test_state, "double", STATE_KEYWORD);
    state_append(test_state, "bool", STATE_KEYWORD);
    state_append(test_state, "char", STATE_KEYWORD);
    state_append(test_state, "signed", STATE_KEYWORD);
    state_append(test_state, "unsigned", STATE_KEYWORD);
    state_append(test_state, "true", STATE_KEYWORD);
    state_append(test_state, "false", STATE_KEYWORD);
    state_append(test_state, "void", STATE_KEYWORD);
    state_append(test_state, "if", STATE_KEYWORD);
    state_append(test_state, "else", STATE_KEYWORD);
    state_append(test_state, "switch", STATE_KEYWORD);
    state_append(test_state, "case", STATE_KEYWORD);
    state_append(test_state, "default", STATE_KEYWORD);
    state_append(test_state, "do", STATE_KEYWORD);
    state_append(test_state, "for", STATE_KEYWORD);
    state_append(test_state, "while", STATE_KEYWORD);
    state_append(test_state, "break", STATE_KEYWORD);
    state_append(test_state, "continue", STATE_KEYWORD);
    state_append(test_state, "return", STATE_KEYWORD);
    state_append(test_state, "goto", STATE_KEYWORD);
    state_append(test_state, "auto", STATE_KEYWORD);
    state_append(test_state, "register", STATE_KEYWORD);
    state_append(test_state, "static", STATE_KEYWORD);
    state_append(test_state, "extern", STATE_KEYWORD);
    state_append(test_state, "const", STATE_KEYWORD);
    state_append(test_state, "volitile", STATE_KEYWORD);
    state_append(test_state, "sizeof", STATE_KEYWORD);
    state_append(test_state, "struct", STATE_KEYWORD);
    state_append(test_state, "union", STATE_KEYWORD);
    state_append(test_state, "enum", STATE_KEYWORD);
    state_append(test_state, "typedef", STATE_KEYWORD);
    state_append(test_state, "inline", STATE_KEYWORD);
    state_append(test_state, "restrict", STATE_KEYWORD);

    state_append(test_state, "(", STATE_SYMBOL);
    state_append(test_state, ")", STATE_SYMBOL);
    state_append(test_state, "[", STATE_SYMBOL);
    state_append(test_state, "]", STATE_SYMBOL);
    state_append(test_state, "{", STATE_SYMBOL);
    state_append(test_state, "}", STATE_SYMBOL);
    state_append(test_state, ";", STATE_SYMBOL);
    state_append(test_state, ":", STATE_SYMBOL);
    state_append(test_state, ".", STATE_SYMBOL);
    state_append(test_state, ",", STATE_SYMBOL);
    state_append(test_state, "*", STATE_SYMBOL);
    state_append(test_state, "->", STATE_SYMBOL);
*/

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
