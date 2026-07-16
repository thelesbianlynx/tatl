#include "mode.h"

#include "colorizer.h"

//
// Language Modes.
//  -> Functions which intialize the modes as needed.
//      -> Create Colorizer States.
//

// Guard to only initialize a mode once.
#define INIT_GUARD(Mode) { static bool __initialized = false; if (__initialized) return &Mode; else __initialized = true; }


// - C Mode - //

static
Mode* c_mode () {
    static Mode mode = { .name = "C", .strict_words = true, .color_capitals = true };
    INIT_GUARD(mode);

    State* state = state_create();
    state_append(state, "//", STATE_LINE_COMMENT);
    state_append(state, "/*", STATE_BEGIN_COMMENT);
    state_append(state, "*/", STATE_END_COMMENT);
    state_append(state, "\"", STATE_STRING);
    state_append(state, "'",  STATE_CHAR);

    state_append(state, "int", STATE_KEYWORD); 
    state_append(state, "long", STATE_KEYWORD);
    state_append(state, "short", STATE_KEYWORD);
    state_append(state, "float", STATE_KEYWORD);
    state_append(state, "double", STATE_KEYWORD);
    state_append(state, "bool", STATE_KEYWORD);
    state_append(state, "char", STATE_KEYWORD);
    state_append(state, "signed", STATE_KEYWORD);
    state_append(state, "unsigned", STATE_KEYWORD);
    state_append(state, "true", STATE_KEYWORD);
    state_append(state, "false", STATE_KEYWORD);
    state_append(state, "void", STATE_KEYWORD);
    state_append(state, "if", STATE_KEYWORD);
    state_append(state, "else", STATE_KEYWORD);
    state_append(state, "switch", STATE_KEYWORD);
    state_append(state, "case", STATE_KEYWORD);
    state_append(state, "default", STATE_KEYWORD);
    state_append(state, "do", STATE_KEYWORD);
    state_append(state, "for", STATE_KEYWORD);
    state_append(state, "while", STATE_KEYWORD);
    state_append(state, "break", STATE_KEYWORD);
    state_append(state, "continue", STATE_KEYWORD);
    state_append(state, "return", STATE_KEYWORD);
    state_append(state, "goto", STATE_KEYWORD);
    state_append(state, "auto", STATE_KEYWORD);
    state_append(state, "register", STATE_KEYWORD);
    state_append(state, "static", STATE_KEYWORD);
    state_append(state, "extern", STATE_KEYWORD);
    state_append(state, "const", STATE_KEYWORD);
    state_append(state, "volitile", STATE_KEYWORD);
    state_append(state, "sizeof", STATE_KEYWORD);
    state_append(state, "struct", STATE_KEYWORD);
    state_append(state, "union", STATE_KEYWORD);
    state_append(state, "enum", STATE_KEYWORD);
    state_append(state, "typedef", STATE_KEYWORD);
    state_append(state, "inline", STATE_KEYWORD);
    state_append(state, "restrict", STATE_KEYWORD);

    state_append(state, "(", STATE_SYMBOL);
    state_append(state, ")", STATE_SYMBOL);
    state_append(state, "[", STATE_SYMBOL);
    state_append(state, "]", STATE_SYMBOL);
    state_append(state, "{", STATE_SYMBOL);
    state_append(state, "}", STATE_SYMBOL);
    //state_append(state, "#", STATE_SYMBOL);
    state_append(state, ";", STATE_SYMBOL);
    state_append(state, ":", STATE_SYMBOL);
    state_append(state, "?", STATE_SYMBOL);
    state_append(state, ".", STATE_SYMBOL);
    state_append(state, ",", STATE_SYMBOL);
    state_append(state, "->", STATE_SYMBOL);

    mode.colorizer_state = state;
    return &mode;
}

// - Cpp Mode - //

// - Java Mode - //

// - Makefile Mode - //

static
Mode* make_mode () {
    static Mode mode = { .name = "Makefile", .force_hard_tabs = true , .color_capitals = true };
    INIT_GUARD(mode)

    State* state = state_create();
    state_append(state, "#",  STATE_LINE_COMMENT);
    state_append(state, "\"", STATE_STRING);
    state_append(state, "'",  STATE_CHAR);

    state_append(state, "include", STATE_KEYWORD); 
    state_append(state, "ifeq", STATE_KEYWORD);
    state_append(state, "ifneq", STATE_KEYWORD);
    state_append(state, "ifdef", STATE_KEYWORD);
    state_append(state, "ifndef", STATE_KEYWORD);
    state_append(state, "else", STATE_KEYWORD);
    state_append(state, "endif", STATE_KEYWORD);
    state_append(state, "define", STATE_KEYWORD);
    state_append(state, "enddef", STATE_KEYWORD);
    state_append(state, "export", STATE_KEYWORD);
    state_append(state, "unexport", STATE_KEYWORD);
    state_append(state, "override", STATE_KEYWORD);

    state_append(state, "wildcard", STATE_KEYWORD);
    state_append(state, "shell", STATE_KEYWORD);
    state_append(state, "subst", STATE_KEYWORD);
    state_append(state, "patsubst", STATE_KEYWORD);
    state_append(state, "foreach", STATE_KEYWORD);
    state_append(state, "filter", STATE_KEYWORD);
    state_append(state, "filter-out", STATE_KEYWORD);
    state_append(state, "eval", STATE_KEYWORD);

    state_append(state, "%", STATE_SYMBOL);
    state_append(state, "$", STATE_SYMBOL);
    state_append(state, "$<", STATE_SYMBOL);
    state_append(state, "$^", STATE_SYMBOL);
    state_append(state, "$?", STATE_SYMBOL);
    state_append(state, "$*", STATE_SYMBOL);
    state_append(state, "@", STATE_SYMBOL);
    state_append(state, "(", STATE_SYMBOL);
    state_append(state, ")", STATE_SYMBOL);
    state_append(state, "[", STATE_SYMBOL);
    state_append(state, "]", STATE_SYMBOL);
    state_append(state, "{", STATE_SYMBOL);
    state_append(state, "}", STATE_SYMBOL);
    state_append(state, ":", STATE_SYMBOL);
    state_append(state, ";", STATE_SYMBOL);
    state_append(state, ",", STATE_SYMBOL);
    state_append(state, "|", STATE_SYMBOL);

    mode.colorizer_state = state;
    return &mode;
}

// - Bash Mode - //


//
// Get Language mode from filename.
//

typedef struct {
    const char* ext;
    Mode* (*mode_fn)();
} ModeExt;


static ModeExt mode_list[] = {
    {".c", c_mode},
    {".h", c_mode},
    {"makefile", make_mode },
    {"Makefile", make_mode },
};

static
const char* get_ext (const char* filename) {
    int32_t i = 0;
    int32_t r = 0;
    while (filename[i] != '\0') {
        if (filename[i] == '.') r = i;
        i++;
    }
    return filename + r;
}

Mode* get_language_mode (const char* filename) {
    const char* ext = get_ext(filename);

    for (int i = 0; i < sizeof(mode_list)/sizeof(ModeExt); i++) {
        ModeExt modex = mode_list[i];
        if (strcmp(ext, modex.ext) == 0) return modex.mode_fn();
    }

    return NULL;
}
