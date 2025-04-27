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
}

#endif // N_LINUX_X11
