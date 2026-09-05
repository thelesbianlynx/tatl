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

static
Mode* cpp_mode () {
    static Mode mode = { .name = "C++", .strict_words = true, .color_capitals = true };
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
    
    state_append(state, "class", STATE_KEYWORD);
    state_append(state, "public", STATE_KEYWORD);
    state_append(state, "private", STATE_KEYWORD);
    state_append(state, "protected", STATE_KEYWORD);
    state_append(state, "this", STATE_KEYWORD);
    state_append(state, "virtual", STATE_KEYWORD);
    state_append(state, "fiend", STATE_KEYWORD);
    state_append(state, "explicit", STATE_KEYWORD);
    state_append(state, "mutable", STATE_KEYWORD);
    state_append(state, "template", STATE_KEYWORD);
    state_append(state, "typename", STATE_KEYWORD);
    state_append(state, "export", STATE_KEYWORD);
    state_append(state, "const_cast", STATE_KEYWORD);
    state_append(state, "static_cast", STATE_KEYWORD);
    state_append(state, "reinterpret_cast", STATE_KEYWORD);
    state_append(state, "dynamic_cast", STATE_KEYWORD);
    state_append(state, "typeid", STATE_KEYWORD);
    state_append(state, "new", STATE_KEYWORD);
    state_append(state, "delete", STATE_KEYWORD);
    state_append(state, "try", STATE_KEYWORD);
    state_append(state, "catch", STATE_KEYWORD);
    state_append(state, "throw", STATE_KEYWORD);
    state_append(state, "noexcept", STATE_KEYWORD);
    state_append(state, "namespace", STATE_KEYWORD);
    state_append(state, "using", STATE_KEYWORD);
    state_append(state, "nullptr", STATE_KEYWORD);
    state_append(state, "constexpr", STATE_KEYWORD);

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


// - Java Mode - //

static
Mode* java_mode () {
    static Mode mode = { .name = "Java", .strict_words = true, .color_capitals = true };
    INIT_GUARD(mode);

    State* state = state_create();
    state_append(state, "//", STATE_LINE_COMMENT);
    state_append(state, "/*", STATE_BEGIN_COMMENT);
    state_append(state, "*/", STATE_END_COMMENT);
    state_append(state, "\"", STATE_STRING);
    state_append(state, "'",  STATE_CHAR);

    state_append(state, "int", STATE_KEYWORD);
    state_append(state, "byte", STATE_KEYWORD);
    state_append(state, "char", STATE_KEYWORD);
    state_append(state, "long", STATE_KEYWORD);
    state_append(state, "short", STATE_KEYWORD);
    state_append(state, "float", STATE_KEYWORD);
    state_append(state, "double", STATE_KEYWORD);
    state_append(state, "boolean", STATE_KEYWORD);
    state_append(state, "true", STATE_KEYWORD);
    state_append(state, "false", STATE_KEYWORD);
    state_append(state, "null", STATE_KEYWORD);
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
    state_append(state, "const", STATE_KEYWORD);
    state_append(state, "goto", STATE_KEYWORD);

    state_append(state, "class", STATE_KEYWORD);
    state_append(state, "interface", STATE_KEYWORD);
    state_append(state, "enum", STATE_KEYWORD);
    state_append(state, "extends", STATE_KEYWORD);
    state_append(state, "implements", STATE_KEYWORD);
    state_append(state, "public", STATE_KEYWORD);
    state_append(state, "private", STATE_KEYWORD);
    state_append(state, "protected", STATE_KEYWORD);
    state_append(state, "final", STATE_KEYWORD);
    state_append(state, "static", STATE_KEYWORD);
    state_append(state, "anstract", STATE_KEYWORD);
    state_append(state, "this", STATE_KEYWORD);
    state_append(state, "super", STATE_KEYWORD);
    state_append(state, "package", STATE_KEYWORD);
    state_append(state, "import", STATE_KEYWORD);
    state_append(state, "new", STATE_KEYWORD);
    state_append(state, "try", STATE_KEYWORD);
    state_append(state, "catch", STATE_KEYWORD);
    state_append(state, "throw", STATE_KEYWORD);
    state_append(state, "throws", STATE_KEYWORD);
    state_append(state, "finally", STATE_KEYWORD);
    state_append(state, "assert", STATE_KEYWORD);
    state_append(state, "instanceof", STATE_KEYWORD);
    state_append(state, "strictfp", STATE_KEYWORD);
    state_append(state, "syncronized", STATE_KEYWORD);
    state_append(state, "transient", STATE_KEYWORD);
    state_append(state, "volatile", STATE_KEYWORD);
    state_append(state, "permits", STATE_KEYWORD);
    state_append(state, "record", STATE_KEYWORD);
    state_append(state, "sealed", STATE_KEYWORD);

    state_append(state, "(", STATE_SYMBOL);
    state_append(state, ")", STATE_SYMBOL);
    state_append(state, "[", STATE_SYMBOL);
    state_append(state, "]", STATE_SYMBOL);
    state_append(state, "{", STATE_SYMBOL);
    state_append(state, "}", STATE_SYMBOL);
    state_append(state, ";", STATE_SYMBOL);
    state_append(state, ":", STATE_SYMBOL);
    state_append(state, "?", STATE_SYMBOL);
    state_append(state, ".", STATE_SYMBOL);
    state_append(state, ",", STATE_SYMBOL);

    mode.colorizer_state = state;
    return &mode;
}


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
    state_append(state, "=", STATE_SYMBOL);

    mode.colorizer_state = state;
    return &mode;
}


// - Bash Mode - //

static
Mode* bash_mode () {
    static Mode mode = { .name = "Shell Script", .color_capitals = true };
    INIT_GUARD(mode)

    State* state = state_create();
    state_append(state, "#",  STATE_LINE_COMMENT);
    state_append(state, "\"", STATE_STRING);
    state_append(state, "'",  STATE_CHAR);

    state_append(state, "if", STATE_KEYWORD);
    state_append(state, "else", STATE_KEYWORD);
    state_append(state, "fi", STATE_KEYWORD);
    state_append(state, "case", STATE_KEYWORD);
    state_append(state, "in", STATE_KEYWORD);
    state_append(state, "esac", STATE_KEYWORD);
    state_append(state, "for", STATE_KEYWORD);
    state_append(state, "while", STATE_KEYWORD);
    state_append(state, "until", STATE_KEYWORD);
    state_append(state, "do", STATE_KEYWORD);
    state_append(state, "done", STATE_KEYWORD);
    state_append(state, "function", STATE_KEYWORD);
    state_append(state, "select", STATE_KEYWORD);
    state_append(state, "time", STATE_KEYWORD);
    state_append(state, "echo", STATE_KEYWORD);
    state_append(state, "read", STATE_KEYWORD);
    state_append(state, "printf", STATE_KEYWORD);
    state_append(state, "coproc", STATE_KEYWORD);

    state_append(state, "$", STATE_SYMBOL);
    state_append(state, "$@", STATE_SYMBOL);
    state_append(state, "$#", STATE_SYMBOL);
    state_append(state, "$?", STATE_SYMBOL);
    state_append(state, "$*", STATE_SYMBOL);
    state_append(state, "$!", STATE_SYMBOL);
    state_append(state, "$_", STATE_SYMBOL);
    state_append(state, "$-", STATE_SYMBOL);
    state_append(state, "$0", STATE_SYMBOL);
    state_append(state, "$1", STATE_SYMBOL);
    state_append(state, "$2", STATE_SYMBOL);
    state_append(state, "$3", STATE_SYMBOL);
    state_append(state, "$4", STATE_SYMBOL);
    state_append(state, "$5", STATE_SYMBOL);
    state_append(state, "$6", STATE_SYMBOL);
    state_append(state, "$7", STATE_SYMBOL);
    state_append(state, "$8", STATE_SYMBOL);
    state_append(state, "$9", STATE_SYMBOL);
    state_append(state, "(", STATE_SYMBOL);
    state_append(state, ")", STATE_SYMBOL);
    state_append(state, "[", STATE_SYMBOL);
    state_append(state, "]", STATE_SYMBOL);
    state_append(state, "{", STATE_SYMBOL);
    state_append(state, "}", STATE_SYMBOL);
    state_append(state, "<", STATE_SYMBOL);
    state_append(state, ">", STATE_SYMBOL);
    state_append(state, ";", STATE_SYMBOL);
    state_append(state, "|", STATE_SYMBOL);
    state_append(state, "&", STATE_SYMBOL);
    state_append(state, "!", STATE_SYMBOL);
    state_append(state, "=", STATE_SYMBOL);
    state_append(state, "=~", STATE_SYMBOL);
    state_append(state, "!=", STATE_SYMBOL);


    mode.colorizer_state = state;
    return &mode;
}


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
    {".cc", cpp_mode},
    {".hh", cpp_mode},
    {".cpp", cpp_mode},
    {".hpp", cpp_mode},
    {".C", cpp_mode},
    {".H", cpp_mode},
    {".java", java_mode},
    {"makefile", make_mode },
    {"Makefile", make_mode },
    {".sh", bash_mode},
    {".profile", bash_mode},
    {".bash_profile", bash_mode},
    {".bashrc", bash_mode},
    {".command", bash_mode},
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
