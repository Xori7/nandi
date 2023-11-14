#ifdef _WINDOWS

#include "nandi.h"
#include <windows.h>

typedef enum {
    N_KEY_DOWN,
    N_KEY_UP,
    N_CURSOR_MOVE,
    N_MOUSE_BUTTON_DOWN,
    N_MOUSE_BUTTON_UP,
    N_MOUSE_WHEEL
} NInputMessageType;

typedef struct {
    bool isPressed;
    bool changed;
} KeyState;

const uint32_t KEY_COUNT = 256;
struct {
    NVec2u32 cursorPosition;
    int32_t mouseWheel;
    KeyState keys[256];
} typedef InputData;

InputData inputData = {0};

void process_input_message(NInputMessageType type, uint32_t data) {
    switch (type) {
        case N_KEY_DOWN:
        case N_MOUSE_BUTTON_DOWN:
            data &= 0xff;
            inputData.keys[data].changed = true;
            inputData.keys[data].isPressed = true;
            break;
        case N_KEY_UP:
        case N_MOUSE_BUTTON_UP:
            data &= 0xff;
            inputData.keys[data].changed = true;
            inputData.keys[data].isPressed = false;
            break;
        case N_CURSOR_MOVE:
            inputData.cursorPosition.x = data & 0xffff;
            inputData.cursorPosition.y = data >> 16;
            break;
        case N_MOUSE_WHEEL:
            inputData.mouseWheel = (int32_t)data;
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

extern bool n_input_key(NKeyCode keyCode) {
    return inputData.keys[keyCode].isPressed;
}

extern bool n_input_key_down(NKeyCode keyCode) {
    return inputData.keys[keyCode].isPressed && inputData.keys[keyCode].changed;
}

extern bool n_input_key_up(NKeyCode keyCode) {
    return !inputData.keys[keyCode].isPressed && inputData.keys[keyCode].changed;
}

extern NVec2u32 n_input_cursor_position() {
    return inputData.cursorPosition;
}

extern int32_t n_input_mouse_wheel() {
    return inputData.mouseWheel;
}

#endif
