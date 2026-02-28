#include "editor.h"

#include "array.h"
#include "charbuffer.h"
#include "rope.h"
#include "filebuffer.h"


void editor_init (Editor* editor) {
    editor->buffers = array_create();
    editor->clipboard = array_create();
    editor->dir = charbuffer_create();

    char* cwd = getcwd(NULL, 0);
    assert(*cwd == '/');
    charbuffer_astr(editor->dir, cwd);
    free(cwd);
}

void editor_fini (Editor* editor) {
    for (int i = 0; i < editor->buffers->size; i++) {
        filebuffer_destroy(editor->buffers->data[i]);
    }
    for (int i = 0; i < editor->clipboard->size; i++) {
        rope_destroy(editor->clipboard->data[i]);
    }
    array_destroy(editor->buffers);
    array_destroy(editor->clipboard);
    charbuffer_destroy(editor->dir);
}


bool editor_event (Editor* editor, InputEvent* event) {

    return true;
}

void editor_draw (Editor* editor) {

}
