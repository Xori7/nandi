#if N_LINUX_X11

#include "nandi/n_input.h"

typedef struct {
    Bool isPressed;
    Bool changed;
} N_KeyState;

const U32 KEY_COUNT = 256;
typedef struct {
    U32 cursor_position_x;
    U32 cursor_position_y;
    I32 mouse_wheel;
    N_KeyState keys[256];
} N_InputData;

N_InputData inputData = {0};

extern void n_input_update() {
    // TODO(kkard2): impl
}

extern Bool n_input_key(N_KeyCode keyCode) {
    // TODO(kkard2): impl
    return FALSE;
}

extern Bool n_input_key_down(N_KeyCode keyCode) {
    // TODO(kkard2): impl
    return FALSE;
}

extern Bool n_input_key_up(N_KeyCode keyCode) {
    // TODO(kkard2): impl
    return FALSE;
}

extern U32 n_input_cursor_position_x() {
    // TODO(kkard2): impl
    return 0;
}
extern U32 n_input_cursor_position_y() {
    // TODO(kkard2): impl
    return 0;
}

extern int32_t n_input_mouse_wheel() {
    // TODO(kkard2): impl
    return 0;
}

#endif // N_LINUX_X11
