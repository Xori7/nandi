#include "nandi.h"
#include <windows.h>

extern void n_input_update() {
    MSG message = {0};
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        switch (message.message) {
            case WM_KEYDOWN:
                break;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}
