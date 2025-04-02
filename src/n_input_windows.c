#define _WINDOWS
#ifdef _WINDOWS

#include "nandi/n_input.h"
#include <windows.h>

typedef enum {
    N_KEY_DOWN,
    N_KEY_UP,
    N_CURSOR_MOVE,
    N_MOUSE_BUTTON_DOWN,
    N_MOUSE_BUTTON_UP,
    N_MOUSE_WHEEL
} N_InputMessageType;

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

void process_input_message(N_InputMessageType type, U32 data) {
    switch (type) {
        case N_KEY_DOWN:
        case N_MOUSE_BUTTON_DOWN:
            data &= 0xff;
            inputData.keys[data].changed = TRUE;
            inputData.keys[data].isPressed = TRUE;
            break;
        case N_KEY_UP:
        case N_MOUSE_BUTTON_UP:
            data &= 0xff;
            inputData.keys[data].changed = TRUE;
            inputData.keys[data].isPressed = FALSE;
            break;
        case N_CURSOR_MOVE:
            inputData.cursor_position_x = data & 0xffff;
            inputData.cursor_position_y = data >> 16;
            break;
        case N_MOUSE_WHEEL:
            inputData.mouse_wheel = (int32_t)data;
            break;
    }
}

extern void n_input_update() {
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        switch(msg.message){
            case WM_KEYDOWN: {
                process_input_message(N_KEY_DOWN, msg.wParam);
                break;
            }
            case WM_KEYUP: {
                process_input_message(N_KEY_UP, msg.wParam);
                break;
            }
            case WM_MOUSEMOVE: {
                process_input_message(N_CURSOR_MOVE, msg.lParam);
                break;
            }
            case WM_LBUTTONDOWN: {
                process_input_message(N_MOUSE_BUTTON_DOWN, 1);
                break;
            }
            case WM_RBUTTONDOWN: {
                process_input_message(N_MOUSE_BUTTON_DOWN, 2);
                break;
            }
            case WM_LBUTTONUP: {
                process_input_message(N_MOUSE_BUTTON_UP, 1);
                break;
            }
            case WM_RBUTTONUP: {
                process_input_message(N_MOUSE_BUTTON_UP, 2);
                break;
            }
            case WM_MOUSEWHEEL: {
                uint32_t data = GET_WHEEL_DELTA_WPARAM(msg.wParam);
                process_input_message(N_MOUSE_WHEEL, data);
                break;
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

extern Bool n_input_key(N_KeyCode keyCode) {
    return inputData.keys[keyCode].isPressed;
}

extern Bool n_input_key_down(N_KeyCode keyCode) {
    return inputData.keys[keyCode].isPressed && inputData.keys[keyCode].changed;
}

extern Bool n_input_key_up(N_KeyCode keyCode) {
    return !inputData.keys[keyCode].isPressed && inputData.keys[keyCode].changed;
}

extern U32 n_input_cursor_position_x() {
    return inputData.cursor_position_x;
}
extern U32 n_input_cursor_position_y() {
    return inputData.cursor_position_y;
}

extern int32_t n_input_mouse_wheel() {
    return inputData.mouse_wheel;
}

#endif
