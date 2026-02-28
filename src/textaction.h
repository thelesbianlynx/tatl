#pragma once

#include "main.h"


//
// Perform a Text-Action according to an input event.
//

void textaction (InputEvent* event, TextBuffer* buffer, int32_t i, Array* clipboard);


//
// Macros to make event handling easier to read.
//

// Begin handle event.
#define ON_KEY(event) switch ((event)->type)

// Control+Key events.
//  - Requires external break.
#define KEY_CTRL(event) case INPUT_CHAR: switch ((event)->charcode)
#define CTRL(key) case key - 'A' + 1:

// Alt+Key events.
//  - Requires external break.
#define KEY_ALT(event) case INPUT_ALT_CHAR: switch ((event)->charcode)
#define ALT(key) case key:

// Other Keys.

#define KEY_ESC case INPUT_ESC:
#define KEY_TAB case INPUT_TAB:
#define KEY_ENTER case INPUT_ENTER:
#define KEY_BACKSPACE case INPUT_BACKSPACE:

#define KEY_SHIFT_TAB case INPUT_SHIFT_TAB:
#define KEY_ALT_ENTER case INPUT_ALT_ENTER:
#define KEY_ALT_BACKSPACE case INPUT_ALT_BACKSPACE:

#define KEY_UP case INPUT_UP:
#define KEY_DOWN case INPUT_DOWN:
#define KEY_LEFT case INPUT_LEFT:
#define KEY_RIGHT case INPUT_RIGHT:

#define KEY_SHIFT_UP case INPUT_SHIFT_UP:
#define KEY_SHIFT_DOWN case INPUT_SHIFT_DOWN:
#define KEY_SHIFT_LEFT case INPUT_SHIFT_LEFT:
#define KEY_SHIFT_RIGHT case INPUT_SHIFT_RIGHT:

#define KEY_ALT_UP case INPUT_ALT_UP:
#define KEY_ALT_DOWN case INPUT_ALT_DOWN:
#define KEY_ALT_LEFT case INPUT_ALT_LEFT:
#define KEY_ALT_RIGHT case INPUT_ALT_RIGHT:

#define KEY_SHIFT_ALT_UP case INPUT_SHIFT_ALT_UP:
#define KEY_SHIFT_ALT_DOWN case INPUT_SHIFT_ALT_DOWN:
#define KEY_SHIFT_ALT_LEFT case INPUT_SHIFT_ALT_LEFT:
#define KEY_SHIFT_ALT_RIGHT case INPUT_SHIFT_ALT_RIGHT:

#define KEY_CTRL_UP case INPUT_CTRL_UP:
#define KEY_CTRL_DOWN case INPUT_CTRL_DOWN:
#define KEY_CTRL_LEFT case INPUT_CTRL_LEFT:
#define KEY_CTRL_RIGHT case INPUT_CTRL_RIGHT:

#define KEY_SHIFT_CTRL_UP case INPUT_SHIFT_CTRL_UP:
#define KEY_SHIFT_CTRL_DOWN case INPUT_SHIFT_CTRL_DOWN:
#define KEY_SHIFT_CTRL_LEFT case INPUT_SHIFT_CTRL_LEFT:
#define KEY_SHIFT_CTRL_RIGHT case INPUT_SHIFT_CTRL_RIGHT:

#define KEY_CTRL_ALT_UP case INPUT_CTRL_ALT_UP:
#define KEY_CTRL_ALT_DOWN case INPUT_CTRL_ALT_DOWN:
#define KEY_CTRL_ALT_LEFT case INPUT_CTRL_ALT_LEFT:
#define KEY_CTRL_ALT_RIGHT case INPUT_CTRL_ALT_RIGHT:

#define KEY_HOME case INPUT_HOME:
#define KEY_END case INPUT_END:

#define KEY_SHIFT_HOME case INPUT_SHIFT_HOME:
#define KEY_SHIFT_END case INPUT_SHIFT_END:

#define KEY_PGUP case INPUT_PGUP:
#define KEY_PGDOWN case INPUT_PGDOWN:

#define KEY_INSERT case INPUT_INSERT:
#define KEY_DELETE case INPUT_DELETE:

#define KEY_F1 case INPUT_F1:
#define KEY_F2 case INPUT_F2:
#define KEY_F3 case INPUT_F3:
#define KEY_F4 case INPUT_F4:
#define KEY_F5 case INPUT_F5:
#define KEY_F6 case INPUT_F6:
#define KEY_F7 case INPUT_F7:
#define KEY_F8 case INPUT_F8:
#define KEY_F9 case INPUT_F9:
#define KEY_F10 case INPUT_F10:
#define KEY_F11 case INPUT_F11:
#define KEY_F12 case INPUT_F12:
